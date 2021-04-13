#include <stdio.h>
#include <std.lib>
#include <math.h>

struct pk_list{
  long sn; //sequence number
  double artm; //sender에서 packet이 발생한 시간
  double t_out; //timeout의 예정 시간
  struct pc_list *link;
}

typedef struct pk_list DataQue;
DataQue WQ_front, WQ_rear; // 발생했지만 아직 전송을 대기 중인 큐
DataQue TransitQ_front, TransitQ_rear // 전송을 하고 ACK가 오기를 기다리는 큐

struct ack_list{
  long sn; //받은 packet의 순서 번호
  double rcvtm; //센더가 이 ACK를 받게 될 시간
  struct ack_list *link;
}

// selective - repeat의 경우에는 sender가 관리하는 큐 외에
// receiver가 관리하는 큐도 구현해야함
// TODO

typedef struct ack_list AckQue;
AckQue AQ_front, AQ_rear; //발생한 ACK에 관한 큐

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
double timeout_len; //얼마나 경과되어야 timeout 될지
int W; //sliding window size
float a; // t_pro / t_pk 한 비율
float t_pk; //하나의 패킷을 전송하는데에 걸리는 시간 (첫번째 비트 출발 ~ 마지막 비트 출발)
float t_pro; // travel하는데에 걸리는 시간
float lamba; // 패킷이 발생하는 rate
float p; //해당되는 패킷이 에러가 날 확률

float random(void);
void pk_gen(double);
void suc_transmission(long);
void re_transmit(void);
void deque_Ack(void);
void enque_Ack(long);
void transmit_pk(void);
void receive_pk(long, double);
void enque_Transmit(void);
void cur_tm_update(void);
void print_performance_measure(void);

void main(void){
  /* input params setting */

  WQ_front = NULL;
  WQ_rear = NULL;
  TransitQ_front = NULL;
  TransitQ_rear = NULL;
  AQ_front = NULL;
  AQ_rear = NULL;
  cur_tm = -log(random()) / lambda;
  pk_gen(cur_tm);
  next_pk_artm = curtm - log(random()) / lambda;

  while (cur_tm <= simul_tm){
    while(AQ_front != NULL){
      if(AQ_front -> rcvtm <= cur_tm)
    }
      if(AQ)
  }



}