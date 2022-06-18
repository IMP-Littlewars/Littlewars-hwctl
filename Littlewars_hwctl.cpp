#include "Littlewars_hwctl.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "HWtest", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "HWtest", __VA_ARGS__))

extern "C" {

    /*
        < LED function >
        0   : Ceremony
        1~8 : LED on
    */
    void gled(int signal) {
        pthread_t thr;
        int thr_id;
        printf("led!, %d\n", signal);
        gled_signal = signal;

        thr_id = pthread_create(&thr, NULL, thread_led, NULL);
        if (thr_id < 0) {
            exit(0);
        }
        pthread_detach(thr);
    }

    /*
        < Piezo function >
        0 : win melody
        1 : def melody
    */
    void gpiezo(int signal) {
        pthread_t thr;
        int thr_id;
        printf("piezo!, %d\n", signal);
        gpiezo_signal = signal;

        thr_id = pthread_create(&thr, NULL, thread_piezo, NULL);
        if (thr_id < 0) {
            exit(0);
        }
        pthread_detach(thr);
    }

    /*
        < Segment_init function >
        default -> thread create
    */
    void gsegment_init(void) {
        pthread_t thr;
        int thr_id;
        printf("segment!\n");

        gsegment_signal = 0;
        thr_id = pthread_create(&thr, NULL, thread_segment, NULL);
        if (thr_id < 0) {
            exit(0);
        }
        pthread_detach(thr);
    }

    /*
        < Segment function >
        0   : init (0,)
        1   : inGame (1, time)
        100 : exit thread (100,)
    */
    void gsegment(int signal, int time) {
        gsegment_signal = signal;
        gsegment_time = time;
    }

    /*
        < 4led function >
        0  : full color
        1  : Red
        2  : Green
        3  : Blue
    */
    void g4led(int signal) {
        char white[3] = { 100, 100, 100 };
        char red[3] = { 100, 0, 0 };
        char green[3] = { 0, 100, 0 };
        char blue[3] = { 0, 0, 100 };

        int fd_4led = open("/dev/fpga_fullcolorled", O_RDWR | O_SYNC);

        switch (signal) {
        case 0:
            ioctl(fd_4led, LED_1);
            write(fd_4led, white, 3);
            ioctl(fd_4led, LED_2);
            write(fd_4led, white, 3);
            ioctl(fd_4led, LED_3);
            write(fd_4led, white, 3);
            ioctl(fd_4led, LED_4);
            write(fd_4led, white, 3);
            break;
        case 1:
            ioctl(fd_4led, LED_1);
            write(fd_4led, red, 3);
            ioctl(fd_4led, LED_2);
            write(fd_4led, red, 3);
            ioctl(fd_4led, LED_3);
            write(fd_4led, red, 3);
            ioctl(fd_4led, LED_4);
            write(fd_4led, red, 3);
            break;
        case 2:
            ioctl(fd_4led, LED_1);
            write(fd_4led, green, 3);
            ioctl(fd_4led, LED_2);
            write(fd_4led, green, 3);
            ioctl(fd_4led, LED_3);
            write(fd_4led, green, 3);
            ioctl(fd_4led, LED_4);
            write(fd_4led, green, 3);
            break;
        case 3:
            ioctl(fd_4led, LED_1);
            write(fd_4led, blue, 3);
            ioctl(fd_4led, LED_2);
            write(fd_4led, blue, 3);
            ioctl(fd_4led, LED_3);
            write(fd_4led, blue, 3);
            ioctl(fd_4led, LED_4);
            write(fd_4led, blue, 3);
            break;
        }
        close(fd_4led);
    }

    /*
        < textLcd function >
        0 : init
        1 : inGame  (1, mHp, eHp,)
        2 : winGame (2, , , end)
        3 : loseGame
    */
    void gtextlcd(int signal, int mHp, int eHp, int end) {
        char msgs[2][20];
        int fd_lcd = open("/dev/fpga_textlcd", O_WRONLY);
        assert(fd_lcd != -1);
        ioctl(fd_lcd, TEXTLCD_INIT);
        ioctl(fd_lcd, TEXTLCD_CLEAR);

        if (signal == 0) {
            sprintf(msgs[0], "< Little  Wars >");
            sprintf(msgs[1], "welcome to play!");
        }
        else if (signal == 1) {
            sprintf(msgs[0], "   My  vs Enemy ");
            sprintf(msgs[1], "   %02d      %02d   ", mHp, eHp);
        }
        else if (signal == 2) {
            sprintf(msgs[0], "    YOU WIN!    ");
            sprintf(msgs[1], " < Point  %03d > ", end);
        }
        else {
            sprintf(msgs[0], "    YOU LOSE    ");
            sprintf(msgs[1], "    cheer up    ");
        }

        ioctl(fd_lcd, TEXTLCD_LINE1);
        write(fd_lcd, msgs[0], strlen(msgs[0]));
        ioctl(fd_lcd, TEXTLCD_LINE2);
        write(fd_lcd, msgs[1], strlen(msgs[1]));

        ioctl(fd_lcd, TEXTLCD_OFF);
        close(fd_lcd);
    }

    /*
        < dotMatrix function >
        0   : thread create
        100 : thread exit
    */
    void gdotmatrix(int signal) {
        pthread_t thr;
        int thr_id;
        printf("dot!, %d\n", signal);

        if (signal == 0) {
            gdotmatrix_signal = 0;
            thr_id = pthread_create(&thr, NULL, thread_dotmatrix, NULL);
            if (thr_id < 0) {
                exit(0);
            }
            pthread_detach(thr);
        }
        else if (signal == 100) {
            gdotmatrix_signal = 100;
        }

    }

    ////////////////////////////////////////////////////////////////
    ///////////////////////////// thread ///////////////////////////
    ////////////////////////////////////////////////////////////////

    void* thread_led(void* _signal) {
        int fd_led = open("/dev/fpga_led", O_WRONLY);
        unsigned char value[9] = { 0, 128, 192, 224, 240, 248, 252, 254, 255 };

        assert(fd_led != -1);

        int signal = gled_signal;
        printf("led_thread!, %d\n", signal);

        if (signal == 0) {
            int i;

            for (i = 1; i < 9; i++) {
                write(fd_led, &(value[i]), sizeof(char));
                usleep(TIME_LED);
            }

            for (i = 8; i >= 0; i--) {
                write(fd_led, &(value[i]), sizeof(char));
                usleep(TIME_LED);
            }

            for (i = 0; i < 2; i++) {
                write(fd_led, &(value[0]), sizeof(char));
                usleep(TIME_LED);
                write(fd_led, &(value[8]), sizeof(char));
                usleep(TIME_LED);
            }
            write(fd_led, &(value[0]), sizeof(char));
        }
        else
            write(fd_led, &(value[signal]), sizeof(char));
        close(fd_led);
        pthread_exit(0);
    }

    void* thread_piezo(void* _signal) {
        int fd_piezo = open("/dev/fpga_piezo", O_WRONLY);
        unsigned char win[15] = { 0x11, 0, 0x21, 0x17, 0x21, 0, 0x15, 0, 0x16, 0x15, 0x14, 0x16, 0x15, 0x15, 0 };
        unsigned char defeat[15] = { 0x15, 0x14, 0x15, 0, 0x14, 0x14, 0, 0, 0x12, 0x11, 0x12, 0, 0x11, 0x11, 0 };
        unsigned char start[31] = { 0x11, 0, 0x11, 0, 0x15, 0, 0x15, 0, 0x16, 0x44, 0x21, 0x16, 0x15, 0, 0, 0, 0x14, 0, 0x14, 0, 0x13, 0, 0x13, 0, 0x12, 0, 0x12, 0, 0x11, 0x11, 0 };

        int signal = gpiezo_signal;

        if (signal == 0) {
            int i;
            for (i = 0; i < 31; i++) {
                write(fd_piezo, &(start[i]), sizeof(char));
                usleep(TIME_PIEZO);
            }
        }
        else if (signal == 1) {
            int i;
            for (i = 0; i < 15; i++) {
                write(fd_piezo, &(win[i]), sizeof(char));
                usleep(TIME_PIEZO);
            }
        }
        else if (signal == 2) {
            int i;
            for (i = 0; i < 15; i++) {
                write(fd_piezo, &(defeat[i]), sizeof(char));
                usleep(TIME_PIEZO);
            }
        }

        close(fd_piezo);
        pthread_exit(0);
    }

    void* thread_segment(void* _signal) {
        int fd_segment = open("/dev/fpga_segment", O_WRONLY);
        char nums[7];
        int i, s;

        assert(fd_segment != -1);

        printf("segment_thread!\n");

        while (1) {
            if (gsegment_signal == 0) {
                for (i = 0; i < 20; i++) {
                    write(fd_segment, ":<000;", 6);
                }
                write(fd_segment, "      ", 6);
                sleep(1);
            }
            else if (gsegment_signal == 1) {
                for (i = 0; i < 20; i++) {
                    s = gsegment_time;
                    sprintf(nums, ":<%03d;", s);
                    write(fd_segment, nums, 6);
                }
                write(fd_segment, "      ", 6);
                sleep(1);
            }
            else if (gsegment_signal == 100) {
                write(fd_segment, "      ", 6);
                break;
            }
            else {
                for (i = 0; i < 20; i++) {
                    write(fd_segment, ":<000;", 6);
                }
                write(fd_segment, "      ", 6);
                sleep(1);
            }
        }

        close(fd_segment);
        pthread_exit(0);
    }

    void* thread_dotmatrix(void* none) {
        int dev, i, j, offset = 20, ch, len;
        char result[600], tmp[2];
        char input[100] = "Little wars~!";

        dev = open("/dev/fpga_dotmatrix", O_WRONLY);
        assert(dev != -1);

        while (1) {
            sleep(1);
            if (gdotmatrix_signal == 100) {
                break;
            }
            len = strlen(input);

            for (j = 0; j < 20; j++)
                result[j] = '0';

            for (i = 0; i < len; i++) {
                ch = input[i];
                ch -= 0x20;

                for (j = 0; j < 5; j++) {
                    sprintf(tmp, "%x%x", font[ch][j] / 16, font[ch][j] % 16);

                    result[offset++] = tmp[0];
                    result[offset++] = tmp[1];
                }
                result[offset++] = '0';
                result[offset++] = '0';
            }

            for (j = 0; j < 20; j++)
                result[offset++] = '0';

            for (i = 0; i < (offset - 18) / 2; i++) {
                for (j = 0; j < 20; j++) {
                    write(dev, &result[2 * i], 20);
                }
            }
        }

        close(dev);
        printf("Program Exit !!\n");
        pthread_exit(0);
    }
}
