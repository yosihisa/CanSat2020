/*
 * gnc.c
 *
 *  Created on: Dec 19, 2019
 *      Author: yosihisa
 */


#include "main.h"
#include "gnc.h"
#include "I2C_sensor.h"

#define T 0.2 //サンプリング周期[s]

unsigned long count;

//待機
void mode1_Standby(cansat_t* data) {
    count = 0;
    motor_Speed(&data->motor, 0, 0);

    //分離したら
    if (data->flightPin == 1) {
        data->press_d_lpf = 600;
        count = 0;
        data->mode = 2;
        return;
    }
}

//降下

void mode2_Descent(cansat_t* data) {
    if (T * count >= 60.0 || data->press_d <= 20) {
        count = 0;
        data->mode = 3;
        return;
    }
    count++;
}

//分離
void mode3_Separation(cansat_t* data) {

    if (count == 0) {
        HAL_GPIO_WritePin(Nichrome_GPIO_Port, Nichrome_Pin, GPIO_PIN_SET);
        data->nichrome = 1;
    }

    if (T * count >= 1.2) {
        HAL_GPIO_WritePin(Nichrome_GPIO_Port, Nichrome_Pin, GPIO_PIN_RESET);
        data->nichrome = 0;
        count = 0;
        data->mode = 4;
        return;
    }
    count++;
}

//回避
void mode4_Avoidance(cansat_t* data) {

    if (count == 0) {
        HAL_GPIO_WritePin(SUB_MODE_GPIO_Port, SUB_MODE_Pin, GPIO_PIN_SET);//SUB Mode PINK
    }

    if (T * count >= 1.0) {
        count = 0;
        data->mode = 5;
        return;
    }

    count++;

}

//キャリブレーション
void mode5_Calibration(cansat_t* data) {
    static long x_max, y_max;
    static long x_min, y_min;

    if (count == 0) {
        motor_Speed(&data->motor, 60, 100);
        x_max = data->compass.x;
        x_min = data->compass.x;
        y_max = data->compass.y;
        y_min = data->compass.y;
    }

    x_max = data->compass.x > x_max ? data->compass.x : x_max;
    x_min = data->compass.x < x_min ? data->compass.x : x_min;
    y_max = data->compass.y > y_max ? data->compass.y : y_max;
    y_min = data->compass.y < y_min ? data->compass.y : y_min;

    if (T * count >= 40.0) {
        struct xyz offset;
        offset.x = (x_max + x_min) * 0.5;
        offset.y = (y_max + y_min) * 0.5;

        set_lsm303_offset(&offset);

        motor_Speed(&data->motor, 0, 0);
        count = 0;
        data->mode = 6;
        return;
    }

    count++;

}

//GNSS誘導
void mode6_GNSS(cansat_t* data) {

    static uint32_t goal_radius;

    if (count == 0) {
        HAL_GPIO_WritePin(SUB_MODE_GPIO_Port, SUB_MODE_Pin, GPIO_PIN_RESET);//SUB Mode RED
        struct gnss_goal goal;
        get_gnssGoal(&goal.latitude, &goal.longitude, &goal.dist);
        goal_radius = goal.dist;
    }

    if (data->gnss.state == 1) {
        if (data->arg < -75) {
            motor_Speed(&data->motor, 100, 30);
        }
        if (-75 <= data->arg && data->arg < -15) {
            motor_Speed(&data->motor, 100, 70);

        }
        if (-15 <= data->arg && data->arg < +15) {
            motor_Speed(&data->motor, 100, 100);

        }
        if (+15 <= data->arg && data->arg < +75) {
            motor_Speed(&data->motor, 70, 100);

        }
        if (+75 <= data->arg) {
            motor_Speed(&data->motor, 30, 100);
        }
    }
    else {
        motor_Speed(&data->motor, 40, 75);
    }

    if (data->gnss.dist < goal_radius) {
        motor_Speed(&data->motor, 0, 0);
        data->mode = 7;
        count = 0;
        return;
    }

    count++;
}

//光学誘導
#define SQUARE  100    //カラーコーンだと認識する最小面積
#define GOAL    1000   //ゴール判定(過去５回の重心の移動距離の2乗の合計がこの値以下だったらゴール)
void mode7_Optical(cansat_t* data) {

    static int xp = 0; //画像検出時の1つ前の回転状態
    static int dc[5] = { 0 };//重心の変化量(過去5回)
    static int xc_p = 0, yc_p = 0;//１つ前の重心
    static unsigned short name_p = 0;
    static unsigned short n = 0;
    static unsigned long  t = 0;

    if (count == 0) {
        HAL_GPIO_WritePin(SUB_MODE_GPIO_Port, SUB_MODE_Pin, GPIO_PIN_RESET);//SUB Mode RED
    }

    //新しい画像が取得できていたら
    if (name_p != data->img.name) {

        name_p = data->img.name;
        t = count;
        //一定以上の大きさの赤が存在したら
        if (data->img.s > SQUARE) {
            //右
            if (data->img.xc < 120) {
                xp = 1;
                motor_Speed(&data->motor, 60, 45);

            }
            //やや右
            if (data->img.xc >= 120 && data->img.xc < 240) {
                xp = 1;
                motor_Speed(&data->motor, 70, 65);

            }
            //中央
            if (data->img.xc >= 240 && data->img.xc < 400) {
                xp = 0;
                motor_Speed(&data->motor, 70, 70);

            }
            //やや左
            if (data->img.xc >= 400 && data->img.xc < 520) {
                xp = -1;
                motor_Speed(&data->motor, 65, 70);

            }
            //左
            if (data->img.xc >= 520) {
                xp = -1;
                motor_Speed(&data->motor, 45, 70);
            }

            //ゴール判定
            dc[4] = dc[3];
            dc[3] = dc[2];
            dc[2] = dc[1];
            dc[1] = dc[0];
            dc[0] = (xc_p - data->img.xc) * (xc_p - data->img.xc) + (yc_p - data->img.yc) * (yc_p - data->img.yc);
            xc_p = data->img.xc;
            yc_p = data->img.yc;
            if (n > 5) {
                int dc_sum = dc[0] + dc[1] + dc[2] + dc[3] + dc[4];
                if (dc_sum < GOAL) {
                    motor_Speed(&data->motor, 0, 0);
                    data->mode = 8;
                    count = 0;
                    return;
                }
            }
            n++;

        }

        //画像中にカラーコーンを検出できなかったら
        else {
            if (xp == 1) {
                motor_Speed(&data->motor, 55, 45);
            }
            else {
            }
        }
    }

    //新しい画像がまだ取得できていなかったら
    else {
        if (T * (count - t) > 1.0) {
            motor_Speed(&data->motor, 0, 0);
        }
    }
    count++;
}

//ゴール
void mode8_Goal(cansat_t* data) {
    if (count == 0) {
        motor_Speed(&data->motor, -50, -50);
    }

    if (T * count >= 6.0) {
        motor_Speed(&data->motor, 0, 0);
    }

    count++;
}
