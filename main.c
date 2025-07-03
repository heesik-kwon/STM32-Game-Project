#include "device_driver.h"
#include <stdlib.h>

#define LCDW 320
#define LCDH 240
#define X_MIN 0
#define X_MAX (LCDW - 1)
#define Y_MIN 0
#define Y_MAX (LCDH - 1)

#define TIMER_PERIOD 10
#define PLAYER_STEP 10
#define PLAYER_SIZE 28
#define ZOMBIE_SIZE 28
#define BOSS_SIZE 36
#define MAX_ZOMBIES 10
#define INIT_HP 5
#define SURVIVE_TICK 600

#define DAY_BACK_COLOR WHITE
#define NIGHT_BACK_COLOR BLACK
#define PLAYER_COLOR RED
#define MISSILE_COLOR ORANGE
#define ITEM_COLOR MAGENTA
#define ZOMBIE_DAY_COLOR1 GREEN
#define ZOMBIE_DAY_COLOR2 BLUE
#define ZOMBIE_NIGHT_COLOR1 YELLOW
#define ZOMBIE_NIGHT_COLOR2 VIOLET
#define BOSS_COLOR VIOLET

#define MISSILE_SPEED 10
#define MISSILE_WIDTH 4
#define MISSILE_HEIGHT 10
#define MISSILE_MAX 5
#define MISSILE_COOLDOWN 3

#define ITEM_SIZE 10
#define DIPLAY_MODE 3

#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20

typedef struct {
    int x, y;
    int w, h;
    unsigned short color;
    int img_type;
} QUERY_DRAW;

typedef struct {
    int x, y;
    int active;
} MISSILE;

typedef struct {
    int x, y;
    int active;
} ITEM;

static QUERY_DRAW player;
static QUERY_DRAW zombies[MAX_ZOMBIES];
static QUERY_DRAW boss;
static int boss_active;
static int boss_hp;

static MISSILE missiles[MISSILE_MAX];
static ITEM item;

static int hp;
static int score;
static int game_tick;
static int stage;
static int zombie_count;
static int alive_zombie_count;
static int fire_cooldown_tick = 0;
static unsigned short background_color;
static int item_spawned = 0;
static int booster_active = 0;         // 부스터 효과 활성화 여부
static int booster_duration_tick = 0;  // 부스터 남은 시간 (tick 단위)
static ITEM booster_item;              // booster 아이템 위치


extern volatile int TIM4_expired;
extern volatile int Jog_key_in;
extern volatile int Jog_key;

extern const tImage player_img;
extern const tImage zombie_img;
extern const tImage creep_zombie_img;
extern const tImage bool_zombie_img;
extern const tImage boss_zombie_img;
extern const tImage hp_img;
extern const tImage booster_img;
extern const tImage shield_img;

void Play_Mario_Gameover(void);
void TIM3_Out_Freq_Generation(unsigned short freq);
void TIM3_Out_Stop(void);
void draw_image(int xs, int ys, tImage *d);

void Draw_Object(QUERY_DRAW* obj) {
    if (obj == &player) {
        draw_image(obj->x, obj->y, &player_img);
    } else if (obj >= zombies && obj < zombies + MAX_ZOMBIES) {
        if (obj->img_type == 0)
            draw_image(obj->x, obj->y, &zombie_img);
        else if (obj->img_type == 1)
            draw_image(obj->x, obj->y, &creep_zombie_img);
        else if (obj->img_type == 2)
            draw_image(obj->x, obj->y, &bool_zombie_img);
        else
            draw_image(obj->x, obj->y, &zombie_img);
    } else if (obj == &boss) {
        draw_image(obj->x, obj->y, &boss_zombie_img);
    } else {
        Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, obj->color);
    }
}

int Check_Collision_Box(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return !(x1 + w1 < x2 || x1 > x2 + w2 || y1 + h1 < y2 || y1 > y2 + h2);
}

void Lcd_Erase_Object(QUERY_DRAW* obj) {
    Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, background_color);
}

void Better_Safe_Spawn_Zombie(int idx) {
    int safe_margin = 50;
    int try_count = 0;
    do {
        zombies[idx].x = rand() % (LCDW - ZOMBIE_SIZE);
        zombies[idx].y = rand() % (LCDH - ZOMBIE_SIZE);
        try_count++;
    } while (Check_Collision_Box(player.x - safe_margin, player.y - safe_margin,
                                 PLAYER_SIZE + 2*safe_margin, PLAYER_SIZE + 2*safe_margin,
                                 zombies[idx].x, zombies[idx].y, ZOMBIE_SIZE, ZOMBIE_SIZE) && try_count < 100);
}

unsigned short Get_Zombie_Color(int idx) {
    if (stage % 2 == 0)
        return (idx % 2 == 0) ? ZOMBIE_NIGHT_COLOR1 : ZOMBIE_NIGHT_COLOR2;
    else
        return (idx % 2 == 0) ? ZOMBIE_DAY_COLOR1 : ZOMBIE_DAY_COLOR2;
}

void Set_Background_Mode(int stage) {
    background_color = (stage % 2 == 0) ? NIGHT_BACK_COLOR : DAY_BACK_COLOR;
    Lcd_Draw_Back_Color(background_color);
}

void Fire_Missile(void) {
    int i;
    for (i = 0; i < MISSILE_MAX; i++) {
        if (!missiles[i].active && fire_cooldown_tick == 0) {
            missiles[i].x = player.x + PLAYER_SIZE;
            missiles[i].y = player.y + player.h/2 - MISSILE_HEIGHT/2;
            missiles[i].active = 1;
            fire_cooldown_tick = MISSILE_COOLDOWN;
            break;
        }
    }
}

void Move_Missiles(void) {
    int i, j;
    for (i = 0; i < MISSILE_MAX; i++) {
        if (!missiles[i].active) continue;

        // 먼저 기존 위치 지우기
        Lcd_Draw_Box(missiles[i].x, missiles[i].y, MISSILE_WIDTH, MISSILE_HEIGHT, background_color);

        // 미사일 이동
        missiles[i].x += MISSILE_SPEED;

        // 화면 밖이면 비활성화 후 지우기
        if (missiles[i].x > LCDW) {
            missiles[i].active = 0;
            continue;
        }

        // 보스와 충돌 체크
        if (boss_active && Check_Collision_Box(missiles[i].x, missiles[i].y, MISSILE_WIDTH, MISSILE_HEIGHT,
                                               boss.x, boss.y, boss.w, boss.h)) {
            missiles[i].active = 0;
            boss_hp--;
            if (boss_hp <= 0) {
                Lcd_Erase_Object(&boss);
                boss_active = 0;
                score += 100;
            }
            continue;
        }

        // 좀비와 충돌 체크
        for (j = 0; j < zombie_count; j++) {
            if (zombies[j].color == background_color) continue;
            if (Check_Collision_Box(missiles[i].x, missiles[i].y, MISSILE_WIDTH, MISSILE_HEIGHT,
                                    zombies[j].x, zombies[j].y, zombies[j].w, zombies[j].h)) {
                missiles[i].active = 0;
                zombies[j].color = background_color;
                Lcd_Erase_Object(&zombies[j]);
                alive_zombie_count--;
                score += 10;
                break;
            }
        }

        // 충돌 후에도 여전히 활성화 상태면 그리기
        if (missiles[i].active) {
            Lcd_Draw_Box(missiles[i].x, missiles[i].y, MISSILE_WIDTH, MISSILE_HEIGHT, MISSILE_COLOR);
        }
    }
}


void Check_Item_Collision(void) {
    // HP 아이템 충돌
    if (item.active && Check_Collision_Box(player.x, player.y, player.w, player.h,
                                           item.x, item.y, hp_img.width, hp_img.height)) {
        item.active = 0;
        hp++;
        Lcd_Draw_Box(item.x, item.y, hp_img.width, hp_img.height, background_color);
    }

    // Booster 아이템 충돌
    if (booster_item.active && Check_Collision_Box(player.x, player.y, player.w, player.h,
                                                   booster_item.x, booster_item.y,
                                                   booster_img.width, booster_img.height)) {
        booster_item.active = 0;
        booster_active = 1;
        booster_duration_tick = 300; // 3초 (TIMER_PERIOD = 10ms, 100Hz 기준)
        Lcd_Draw_Box(booster_item.x, booster_item.y, booster_img.width, booster_img.height, background_color);
    }
}


void Randomize_All_Zombies(void) {
    int i;
    for (i = 0; i < zombie_count; i++) {
        Better_Safe_Spawn_Zombie(i);
        zombies[i].w = ZOMBIE_SIZE;
        zombies[i].h = ZOMBIE_SIZE;
        zombies[i].img_type = rand() % 3;
        zombies[i].color = Get_Zombie_Color(i);
        Draw_Object(&zombies[i]);
    }
}

int Move_Zombies(void) {
    int i;
    for (i = 0; i < zombie_count; i++) {
        if (zombies[i].color == background_color) continue;
        Lcd_Erase_Object(&zombies[i]);
        int speed = 2 + stage;
        if (zombies[i].x < player.x) zombies[i].x += speed;
        else if (zombies[i].x > player.x) zombies[i].x -= speed;
        if (zombies[i].y < player.y) zombies[i].y += speed;
        else if (zombies[i].y > player.y) zombies[i].y -= speed;
        zombies[i].color = Get_Zombie_Color(i);
        Draw_Object(&zombies[i]);
        if (Check_Collision_Box(player.x, player.y, player.w, player.h,
                                zombies[i].x, zombies[i].y, zombies[i].w, zombies[i].h)) {
            hp--;
            if (hp <= 0) {
                Lcd_Printf(84, 100, RED, background_color, 2, 2, "GAME OVER");
                Play_Mario_Gameover();
                return 1;
            }
        }
    }
    if (boss_active) {
        int old_x = boss.x;
        int old_y = boss.y;
        int speed = 1 + stage / 2;
        if (boss.x < player.x) boss.x += speed;
        else if (boss.x > player.x) boss.x -= speed;
        if (boss.y < player.y) boss.y += speed;
        else if (boss.y > player.y) boss.y -= speed;
        Lcd_Draw_Box(old_x, old_y, boss.w, boss.h, background_color);
        Draw_Object(&boss);
        if (Check_Collision_Box(player.x, player.y, player.w, player.h,
                                boss.x, boss.y, boss.w, boss.h)) {
            hp -= 2;
            if (hp <= 0) {
                Lcd_Printf(84, 100, RED, background_color, 2, 2, "GAME OVER");
                Play_Mario_Gameover();
                return 1;
            }
        }
    }
    return 0;
}

void Game_Init(void) {
    int i;
    srand(TIM2->CNT ^ (rand() << 8) ^ (TIM2->CNT >> 3));
    hp = INIT_HP;
    score = 0;
    game_tick = 0;
    stage = 1;
    zombie_count = 1;
    alive_zombie_count = zombie_count;
    item.active = 0;
    fire_cooldown_tick = 0;
    boss_active = 0;
    booster_active = 0;
    booster_duration_tick = 0;
    booster_item.active = 0;

    Set_Background_Mode(stage);

    player.x = LCDW / 2;
    player.y = LCDH / 2;
    player.w = PLAYER_SIZE;
    player.h = PLAYER_SIZE;
    player.color = PLAYER_COLOR;
    Draw_Object(&player);

    for (i = 0; i < MAX_ZOMBIES; i++) {
        Better_Safe_Spawn_Zombie(i);
        zombies[i].w = ZOMBIE_SIZE;
        zombies[i].h = ZOMBIE_SIZE;
        zombies[i].img_type = rand() % 3;
        zombies[i].color = background_color;
    }
    Randomize_All_Zombies();
    for (i = 0; i < MISSILE_MAX; i++) missiles[i].active = 0;
}

void System_Init(void) {
    Clock_Init();
    LED_Init();
    Key_Poll_Init();
    Uart1_Init(115200);
    SCB->VTOR = 0x08003000;
    SCB->SHCSR = 7 << 16;
}

void Play_Mario_Gameover(void) {
    int i;
    const unsigned short mario[] = {659, 587, 554, 523, 494, 440, 415};
    for (i = 0; i < 7; i++) {
        TIM3_Out_Freq_Generation(mario[i]);
        TIM2_Delay(200);
    }
    TIM3_Out_Stop();
}

void Main(void) {
    System_Init();
    Uart_Printf("Zombie Survival Game\n");
    Lcd_Init(DIPLAY_MODE);
    Jog_Poll_Init();
    Jog_ISR_Enable(1);
    Uart1_RX_Interrupt_Enable(1);

    while (1) {
        Game_Init();
        TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD * 10);

        while (1) {
            int game_over = 0;

            if (Jog_key_in) {
                Lcd_Draw_Box(player.x, player.y, player.w, player.h, background_color);
                int move_unit = booster_active ? PLAYER_STEP * 2 : PLAYER_STEP;

                switch (Jog_key) {
                    case 0: if (player.y > Y_MIN) player.y -= move_unit; break;
                    case 1: if (player.y + player.h < Y_MAX) player.y += move_unit; break;
                    case 2: if (player.x > X_MIN) player.x -= move_unit; break;
                    case 3: if (player.x + player.w < X_MAX) player.x += move_unit; break;
                    case 4: Fire_Missile(); break;
                }
                Draw_Object(&player);
                Jog_key_in = 0;
            }

            if (TIM4_expired) {
                game_tick++;
                if (fire_cooldown_tick > 0) fire_cooldown_tick--;

                if (booster_active) {
                    booster_duration_tick--;
                    if (booster_duration_tick <= 0) booster_active = 0;
                }

                Move_Missiles();
                game_over = Move_Zombies();
                Check_Item_Collision();

                Lcd_Printf(0, 0, BLUE, background_color, 2, 2, "HP:%d S:%d", hp, score);
                Lcd_Printf(200, 0, RED, background_color, 2, 2, "STAGE %d", stage);

                // 실제 살아있는 좀비 수 확인
                int zombie_alive = 0;
                int i;
                for (i = 0; i < zombie_count; i++) {
                    if (zombies[i].color != background_color)
                        zombie_alive++;
                }

                // 스테이지 클리어 조건
                if (zombie_alive == 0 && !boss_active) {
                    Lcd_Printf(50, 100, GREEN, background_color, 2, 2, "Stage %d CLEAR!", stage);
                    TIM2_Delay(1000);
                    Lcd_Draw_Box(32, 100, 256, 32, background_color);

                    stage++;
                    zombie_count++;
                    alive_zombie_count = zombie_count;
                    Set_Background_Mode(stage);
                    item_spawned = 0;
                    booster_active = 0;
                    booster_duration_tick = 0;

                    if (rand() % 100 < 60) {
                        item.x = rand() % (LCDW - hp_img.width);
                        item.y = rand() % (LCDH - hp_img.height);
                        item.active = 1;
                        draw_image(item.x, item.y, &hp_img);
                        item_spawned = 1;
                    }

                    if (rand() % 100 < 60) {
                        booster_item.x = rand() % (LCDW - booster_img.width);
                        booster_item.y = rand() % (LCDH - booster_img.height);
                        booster_item.active = 1;
                        draw_image(booster_item.x, booster_item.y, &booster_img);
                    }

                    Randomize_All_Zombies();

                    // 보스 등장 조건: 3의 배수 + 좀비 초기화 직후 + 아직 보스 없을 때
                    if (stage % 3 == 0 && !boss_active && alive_zombie_count == zombie_count) {
                        Lcd_Printf(80, 100, RED, background_color, 2, 2, "BOSS STAGE");
                        TIM2_Delay(1000);
                        Lcd_Draw_Box(80, 100, 160, 32, background_color);

                        boss.x = rand() % (LCDW - BOSS_SIZE);
                        boss.y = rand() % (LCDH - BOSS_SIZE);
                        boss.w = BOSS_SIZE;
                        boss.h = BOSS_SIZE;
                        boss_hp = stage;
                        boss_active = 1;
                        Draw_Object(&boss);
                    }
                }

                // 생존 시간 도달
                if (game_tick >= SURVIVE_TICK) {
                    Lcd_Printf(50, 100, GREEN, background_color, 2, 2, "YOU SURVIVED!");
                    break;
                }

                TIM4_expired = 0;
            }

            // 게임 오버 처리
            if (game_over) {
                TIM4_Repeat_Interrupt_Enable(0, 0);
                Uart_Printf("Press JOG to retry\n");
                Jog_Wait_Key_Pressed();
                Jog_Wait_Key_Released();
                break;
            }
        }
    }
}