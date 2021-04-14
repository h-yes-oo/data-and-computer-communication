#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define _CRT_SECURE_NO_WARNINGS   

struct pk_list{
  long sn; //sequence number
  double artm; //sender에서 packet이 발생한 시간
  double t_out; //timeout의 예정 시간
  struct pk_list *link;
};

typedef struct pk_list DataQue;

// 발생했지만 아직 전송을 대기 중인 큐
DataQue* WQ_front;
DataQue* WQ_rear;

// 전송을 하고 ACK가 오기를 기다리는 큐
DataQue* TransitQ_front;
DataQue* TransitQ_rear;

struct ack_list{
  long sn; //받은 packet의 순서 번호
  double rcvtm; //센더가 이 ACK를 받게 될 시간
  struct ack_list *link;
};

// selective - repeat의 경우에는 sender가 관리하는 큐 외에
// receiver가 관리하는 큐도 구현해야함
// TODO

typedef struct ack_list AckQue;
//발생한 ACK에 관한 큐
AckQue* AQ_front;
AckQue* AQ_rear;

long seq_n = 0; //패킷의 순서번호
long transit_pknum = 0; //전송했지만 ack를 받지 못한 개수
long next_acksn = 0; //리시버 입장에서 다음에 기대하는 ACK 넘버 (ACK 보낼 때 1씩 증가)
double cur_tm; //현재 시간을 계속 관리하는 변수
double next_pk_artm; //다음에 도달하는 패킷은 이 시간에 도달할 것으로 예상
double t_pknum=0; //지금까지 성공적으로 전송한 패킷의 수
double t_delay=0; //패킷이 도달하는 데에 걸린 딜레이를 누적
// 평균 딜레이 = t_delay / t_pknum


//Input Parameter
double simul_tm; //시뮬레이션 최종 기간
long N = 10000;
double timeout_len; //얼마나 경과되어야 timeout 될지
int W; //sliding window size
float a; // t_pro / t_pk 한 비율
float t_pk; //하나의 패킷을 전송하는데에 걸리는 시간 (첫번째 비트 출발 ~ 마지막 비트 출발)
float t_pro; // travel하는데에 걸리는 시간
float lambda; // 패킷이 발생하는 rate
float p; //해당되는 패킷이 에러가 날 확률

int user_W = 8;
float user_defined_a = 1000;
float user_lambda = 10;
float user_p = 0.2;

//0과 1사이에 난수 발생
float random_gen(void){
    float rn;
    rn = ((float) rand() / (RAND_MAX));
    return(rn);
};

//tm은 패킷이 생성된 시간; 패킷을 새로 만들어라
void pk_gen(double tm){
    DataQue *ptr;
    
    ptr = malloc(sizeof(DataQue));
    ptr->sn = seq_n;
    ptr->artm = tm;
    ptr->link = NULL;
    seq_n++;
    
    if(WQ_front == NULL)
        WQ_front = ptr;
    else
        WQ_rear -> link = ptr;
    WQ_rear = ptr;
};

//ACK를 성공적으로 전송, sn은 전송한 순서번호
void suc_transmission(long sn){
    DataQue *ptr;
    
    ptr = TransitQ_front;
    //ACK의 순서번호와 transitQ 맨 앞의 순서번호가 같으면
    if(ptr->sn == sn){
        //transitQ에서 하나 뺀다
        TransitQ_front = TransitQ_front->link;
        if(TransitQ_front == NULL)
            TransitQ_rear = NULL;
        transit_pknum--;
        free(ptr);
    }
};

//ACK가 timeout이 되면 transitQ를 모두 waitngQ의 맨 앞으로 보낸다
void re_transmit(void){
    DataQue *ptr;
    
    ptr = TransitQ_front;
    while(ptr != NULL){
        transit_pknum--;
        ptr = ptr->link;
    }
    TransitQ_rear->link = WQ_front;
    if(WQ_front == NULL)
        WQ_rear = TransitQ_rear;
    WQ_front=TransitQ_front;
    TransitQ_front = NULL;
    TransitQ_rear = NULL;
    
};

//Ack Q 맨 앞 원소 뺀다
void deque_Ack(void){
    AckQue *pt;
    
    pt = AQ_front;
    AQ_front = pt->link;
    free(pt);
    if(AQ_front == NULL) AQ_rear = NULL;
};

//Ack 하나 생성
void enque_Ack(long seqn){
    AckQue *ack_ptr;
    
    ack_ptr = malloc(sizeof(AckQue));
    ack_ptr->sn = seqn;
    //ACK를 센더 쪽에서 수신할 것으로 예상되는 시간 = 현재 시간 + 수신 시간 + ACK 발신 시간
    ack_ptr->rcvtm = cur_tm + 2*t_pro;
    ack_ptr->link = NULL;
    
    if(AQ_front == NULL)
        AQ_front = ack_ptr;
    else
        AQ_rear->link = ack_ptr;
    AQ_rear = ack_ptr;
};

//waitingQ에서 transitQ로 enque
void enque_Transmit(void){
    DataQue* ptr;
    ptr = WQ_front;
    transit_pknum++;
    
    WQ_front = WQ_front->link;
    if(WQ_front == NULL) WQ_rear=NULL;
    
    if(TransitQ_front == NULL)
        TransitQ_front = ptr;
    else TransitQ_rear->link = ptr;
    ptr->link = NULL;
    TransitQ_rear = ptr;
};

//패킷 전송 : 유일하게 cur_tm 증가시킨다, timeout 설정한 후 transitQ의 뒤에 넣는다
void transmit_pk(void){
    cur_tm += t_pk;
    WQ_front->t_out = cur_tm + timeout_len;
    enque_Transmit();
};

//seqn : 막 전송된 패킷의 순서번호와 artm
void receive_pk(long seqn, double tm){
    //p 확률로 랜덤하게 에러가 발생
    if(random_gen() > p)
        //리시버가 기대하던 패킷 번호와 동일하면
        if(next_acksn == seqn) {
            //cur_tm + t_pro : 리시브한 시간
            //tm : 패킷이 처음 생성된 시간
            //즉, 패킷이 생성된 시간부터 리시브된 시간까지의 delay
            t_delay += cur_tm + t_pro - tm;
            t_pknum++;
            next_acksn++;
            //AckQ에 하나 넣는다
            enque_Ack(seqn);
        }
};

//next 시간 업데이트
//timeout, packet도착, ACK rcv 추가로 시간 업데이트
void cur_tm_update(void){
    double tm;
    //new packet 도착 시간과 ACK가 센더에 도착한 시간 중 짧은 것을 tm으로
    if(AQ_front == NULL)
        tm = next_pk_artm;
    else if (AQ_front->rcvtm < next_pk_artm)
        tm=AQ_front->rcvtm;
    else tm = next_pk_artm;
    //timeout된 시간이 더 짧으면 tm으로
    if(TransitQ_front != NULL)
        if(TransitQ_front->t_out < tm)
            tm = TransitQ_front->t_out;
    //tm이 cur_tm보다 뒤에 발생하고, 전송을 못할 상황이면 cur_tm을 tm으로 업데이트
    if((tm > cur_tm) && ((WQ_front==NULL) || (transit_pknum >= W)))
        cur_tm = tm;
};

void print_performance_measure(void){
    double util;
    double m_delay;
    
    //평균 딜레이
    m_delay = t_delay/t_pknum;
    //utilization
    util = (t_pknum*t_pk)/cur_tm;

    char result[100];   // Use an array which is large enough 
    snprintf(result, 100, "%lf, %lf\n", m_delay, util);
    printf(result);

    char filename[200];   // Use an array which is large enough 
    snprintf(filename, 200, "/Users/hyesoo/Desktop/practice-cpp/hw3/Go-back-N-a%.0f-p%.3f-W%d-l%.3f.txt", user_defined_a,user_p,user_W,user_lambda);

    FILE *fp = fopen(filename, "a");

    fputs(result, fp);   // 파일에 문자열 저장

    fclose(fp);    // 파일 포인터 닫기
};

void simulate(int inp3, float inp5, float inp6, float inp7){
    /*TODO : input params setting*/
    //simul_tm
    //timeout_len
    //W
    //t_pk, t_pro, a 중 두개
    //lambda, p
    //입력으로 받거나, define으로 하거나, 초기값 설정하거나 알아서 !!
    N = 10000;
    timeout_len = 100;
    W = inp3;
    t_pro = 10;
    a = inp5;
    t_pk = t_pro / a;
    lambda = inp6;
    p = inp7;
    t_pknum = 0;
    
    WQ_front = NULL;
    WQ_rear = NULL;
    TransitQ_front = NULL;
    TransitQ_rear = NULL;
    AQ_front = NULL;
    AQ_rear = NULL;
    
    cur_tm = -log(random_gen()) / lambda; //첫번째 패킷이 생성된 시간을 현재 시간으로 설정
    pk_gen(cur_tm); //지금 막 생성된 패킷을 Waiting Queue의 맨 뒤에 집어 넣는다
    next_pk_artm = cur_tm - log(random_gen()) / lambda; // 다음 패킷이 생성되는 시간을 계산을 현재 시간에 더하여 다음 시간으로 설정
    
    //while (cur_tm <= simul_tm)
    //또는 (t_pknum <= N) 으로 조건 설정 N=10000 최소
    while (t_pknum <= N){
        //ACK Que가 비어 있지 않으면 처리 필요
        while(AQ_front != NULL){
            //현재 시간 전에 ACK가 도달하는 경우
            if(AQ_front -> rcvtm <= cur_tm){
                //성공적인 ACK 전송
                suc_transmission(AQ_front->sn);
                //ACK 하나 뺀다
                deque_Ack();
            }
            else break;
        }
        
        //보낸 큐의 timeout이 지나지 않았는지 확인
        if(TransitQ_front != NULL)
            if(TransitQ_front->t_out <= cur_tm)
                //transitQ에 있는 걸 몽땅 waitingQ로 옮긴다
                re_transmit();
        
        //다음 패킷이 발생했으면,
        while(next_pk_artm <= cur_tm){
            //그 시간을 기점으로 패킷 생성하여 waiting Q에 넣는다
            pk_gen(next_pk_artm);
            //다음 예정시간 갱신
            next_pk_artm += -log(random_gen())/lambda;
        }
        
        //실제 전송은 여기서만 일어남
        //WQ가 비지 않았고 아직 전송할 여유가 있다면
        if((WQ_front != NULL) && (transit_pknum < W)) {
            //제일 앞 패킷 전송하고 타임 업데이트
            transmit_pk();
            //리시버가 패킷 받아서 에러 났는지 안났는지 확인 후 에러 안났으면 ACK 발생 후 ACK Q에 집어 넣는다
            receive_pk(TransitQ_rear->sn, TransitQ_rear->artm);
        }
        
        cur_tm_update();
        
    } //simulation ends
    
    print_performance_measure();
};

int main(void){
    unsigned int seed = (unsigned int) time(NULL);
    srand(seed);
    simulate(user_W, user_defined_a, user_lambda, user_p);
    return 0;
}
