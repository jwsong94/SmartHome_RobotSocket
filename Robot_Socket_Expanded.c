#include <math.h>
#include <stdio.h>
#include <wiringPi.h>
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
//소켓 프로그래밍에 사용될 헤더파일 선언
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_LEN 128

#define A_F 21
#define A_B 22
#define B_F 23
#define B_B 24
#define C_F 25
#define C_B 26
#define D_F 27
#define D_B 28

#define PWM_PIN 1
#define M_SPEED 400

#define FF 11 // Forward
#define BB 22 // Backward
#define SS 33 // Stop

#define U 10 // Up
#define D 20 // Down
#define L 30 // Left
#define R 40 // Right
#define S 50 // Stop
#define TL 60 // Turn Left
#define TR 70 // Turn Right
#define BL 80 // Back Left
#define BR 90 // Back Right

#define SONAR 29 // g20 - p 40

#define STARTX 50
#define STARTY 400
#define STARTA 0

int route[1000][2];
int movecnt=0;
int inputflag=0;
struct timeval start, finish;
long mtime, seconds, useconds;

int client_fd;
double nowX = STARTX;
double nowY = STARTY;
double nowA = STARTA;
char position[8] = "050/400";

void setState(int pos, int st){
    switch(st){
        case FF :
            digitalWrite(pos, 1);
            digitalWrite(pos+1, 0);
            break;
        case BB :
            digitalWrite(pos, 0);
            digitalWrite(pos+1, 1);
            break;
        case SS :
            digitalWrite(pos, 0);
            digitalWrite(pos+1, 0);
            break;
        default : break;
    }
}

void setDirection(int dr){
    //double tempA=cos();
    //double tempB=sin(M_PI);
    if(inputflag==1){
        gettimeofday(&finish, NULL);
        seconds = finish.tv_sec - start.tv_sec;
        useconds = finish.tv_usec - start.tv_usec;
        mtime = (seconds * 1000 + useconds/1000.0)+0.5;
        route[movecnt][1] = (int)mtime;
        switch(route[movecnt][0]){
            case 1: 
                nowX += mtime*100*cos(nowA)/2250;
                nowY -= mtime*100*sin(nowA)/2250;
                break;
            case 2: nowA += mtime*M_PI/2200; break;
            case 3: nowA -= mtime*M_PI/2200; break;
        }
        position[0] = '0' + ((int)nowX)/100;
        position[1] = '0' + (((int)nowX)%100)/10;
        position[2] = '0' + ((int)nowX)%10;
        position[4] = '0' + ((int)nowY)/100;
        position[5] = '0' + (((int)nowY)%100)/10;
        position[6] = '0' + ((int)nowY)%10;
        printf("%s\n", position);
        write(client_fd, position, 8);
        movecnt++;
        inputflag=0;
    }
    switch(dr){
        case U :
            inputflag=1;
            route[movecnt][0]=1;
            setState(A_F,SS);
            setState(B_F,FF);
            setState(C_F,SS);
            setState(D_F,BB);
            gettimeofday(&start, NULL);
            printf("ABCD : SFSB\n");
            break;
        case D :
            setState(A_F,SS);
            setState(B_F,BB);
            setState(C_F,SS);
            setState(D_F,FF);
            printf("ABCD : SBSF\n");
            break;
        case L :
            inputflag=1;
            route[movecnt][0]=2;
            setState(A_F,FF);
            setState(B_F,FF);
            setState(C_F,FF);
            setState(D_F,FF);
            gettimeofday(&start, NULL);
            printf("ABCD : FFFF\n");
            break;
        case R :
            inputflag=1;
            route[movecnt][0]=3;
            setState(A_F,BB);
            setState(B_F,BB);
            setState(C_F,BB);
            setState(D_F,BB);
            gettimeofday(&start, NULL);
            printf("ABCD : BBBB\n");
            break;
        case S :
            setState(A_F,SS);
            setState(B_F,SS);
            setState(C_F,SS);
            setState(D_F,SS);
            printf("ABCD : SSSS\n");
            break;
        case TL :
            setState(A_F,FF);
            setState(B_F,FF);
            setState(C_F,FF);
            setState(D_F,FF);
            printf("ABCD : FFFF\n");
            break;
        case TR :
            setState(A_F,BB);
            setState(B_F,BB);
            setState(C_F,BB);
            setState(D_F,BB);
            printf("ABCD : BBBB\n");
            break;
        case BR :
            setState(A_F,FF);
            setState(B_F,FF);
            setState(C_F,FF);
            setState(D_F,FF);
            break;
        case BL :
            setState(A_F,BB);
            setState(B_F,BB);
            setState(C_F,BB);
            setState(D_F,BB);
            break;
defualt : break;
    }
}

int gpio_init(){
    if(wiringPiSetup() == -1)
        return -1;

    printf("WiringPi Setup END\n");

    pinMode (PWM_PIN, PWM_OUTPUT);
    pinMode (A_F, OUTPUT);
    pinMode (A_B, OUTPUT);
    pinMode (B_F, OUTPUT);
    pinMode (B_B, OUTPUT);
    pinMode (C_F, OUTPUT);
    pinMode (C_B, OUTPUT);
    pinMode (D_F, OUTPUT);
    pinMode (D_B, OUTPUT);

    pwmWrite(PWM_PIN, M_SPEED);

    printf("Pin Mode Setting End\n");
    setDirection(S);
    return 1;
}

void comeback(){
    int i;
    for(i=movecnt; i>=0; i--){
        switch(route[i][0]){
            case 1 : setDirection(D); break;
            case 2 : setDirection(BL); break;
            case 3 : setDirection(BR); break;
        }
        delay(route[i][1]);
        setDirection(S);
        delay(250);
    }
    movecnt=0;
    nowX = STARTX;
    nowY = STARTY;
    nowA = STARTA;
    position[0] = '0';
    position[1] = '5';
    position[2] = '0';
    position[4] = '4';
    position[5] = '0';
    position[6] = '0';
    printf("%s\n", position);
    write(client_fd, position, 8);
}

int main()
{
    int s, n;

    char buffer[BUF_LEN];
    struct sockaddr_in server_addr, client_addr;
    char temp[20];
    int server_fd;
    //server_fd, client_fd : 각 소켓 번호
    int len, msg_size;

    if(gpio_init()==-1){
        printf("gpio init fail\n");
        exit(0);
    }

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {// 소켓 생성
        printf("Server : Can't open stream socket\n");
        exit(0);
    }
    memset(&server_addr, 0x00, sizeof(server_addr));
    //server_Addr 을 NULL로 초기화

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(7000);
    //server_addr 셋팅

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0)
    {//bind() 호출
        printf("Server : Can't bind local address.\n");
        exit(0);
    }

    if(listen(server_fd, 5) < 0)
    {//소켓을 수동 대기모드로 설정
        printf("Server : Can't listening connect.\n");
        exit(0);
    }

    memset(buffer, 0x00, sizeof(buffer));
    printf("Server : wating connection request.\n");
    len = sizeof(client_addr);
    while(1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if(client_fd < 0)
        {
            printf("Server: accept failed.\n");
            exit(0);
        }
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
        printf("Server : %s client connected.\n", temp);

        msg_size = read(client_fd, buffer, 1024);
        printf("%s\n", buffer);
        //write(client_fd, buffer, msg_size);
        
        //write(client_fd, "Test", 5);

        switch(buffer[0])
        {
            case 'u' : setDirection(U); break;
            case 'd' : setDirection(D); break;
            case 'l' : setDirection(L); break;
            case 'r' : setDirection(R); break;
            case 's' : setDirection(S); break;
            case 'b' : comeback(); break;
            default : break;
        }
    }
    close(server_fd);
    return 0;
}

