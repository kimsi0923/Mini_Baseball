#define F_CPU 16000000UL	//16MHz로 설정
#include <avr/io.h>	//레지스터 관련 헤더
#include <util/delay.h>	//딜레이 관련 헤더
#include <avr/interrupt.h>	//인터럽트 관련 헤더
#include <stdlib.h>	//표준 라이브러리 함수 헤더

void UART0_init(void);	//유아트 설정 함수
void UART0_transmit(char data);	//유아트 송신 함수
unsigned char UART0_receive(void);	//유아트 수신 함수

uint8_t numbers[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x67 };	//세그먼트 숫자 									LED 배열
int playerset = 0, chcnt = 0, state = 0;
int pt1 = 0;
int pt2 = 0;
int strike = 0;
int ball = 0;
int out = 0;
int count = 1 , change = 0;
unsigned char ballch;
volatile int hit = 0;

void UIball(void)	//컴퓨터 화면에 볼 상태와 회차를 출력하는 함수
{
	int n = ( chcnt % 2 ) + 49;	//n은 초, 말을 나타냄 1, 2 값을 가진다.chcnt는 공수교대마다 증가한				다. +49는 아스키코드로 표현하기위해 더함
	int i = (( chcnt + 2 ) / 2 ) + 48 ;	//i는 몇회인지 나타냄 , 한 회에 2경기(초, 말)가 끝나야 다음 					회차 이므로 2를 나눈 값에 아스키코드로 표현하기 위해 +48
	if( chcnt < 6 )	//chcnt가 6미만이면, 즉 경기 3회 말까지
	{
		UART0_transmit(i);	//몇 회인지 컴퓨터로 송신 출력
		UART0_transmit('-');	//'-' 출력
		UART0_transmit(n);	//초, 말을 1과 2로 표현, 송신 출력
		UART0_transmit('\n');	
		UART0_transmit('\r');	//깔끔하게 보이기위해 오프셋을 다음줄 시작점으로
	}
	UART0_transmit('B');
	UART0_transmit('A');
	UART0_transmit('L');
	UART0_transmit('L');
	UART0_transmit(' ');
	UART0_transmit('n');
	UART0_transmit('u');
	UART0_transmit('m');
	UART0_transmit(' ');
	UART0_transmit(':');
	UART0_transmit(' ');	//BALL num : 이 나오게 송신 출력
}
void UI(void)	//컴퓨터로 화면이 나오게 출력하는 함수 ( 전광판 역할 )
{
	char bch, strikech, outch;	//ball,strike,out 을 문자로 표현하기 위한 변수
	char pt1ch, pt2ch;	//포인트1, 2를 문자로 표현하기 위한 변수

	bch = ball + 48;	//bch에 ball의 값을 아스키코드로 변환 저장
	strikech = strike + 48;	//strikech에 strike의 값을 아스키코드로 변환 저장
	outch = out + 48;	//outch에 out값을 아스키코드로 변환 저장
	pt1ch = pt1 + 48;	//pt1ch에 pt1값을 아스키코드로 변환 저장
	pt2ch= pt2 + 48;	//pt2ch에 pt2값을 아스키코드로 변환 저장

	UART0_transmit('\n');
	UART0_transmit('\r');
	UART0_transmit('P');
	UART0_transmit('1');
	UART0_transmit('\t');
	if( playerset == 0 )	//만약 player1 순서이면
	{
		UART0_transmit(strikech);
		UART0_transmit('S');
		UART0_transmit(' ');
		UART0_transmit(' ');
		UART0_transmit(bch);
		UART0_transmit('B');
		UART0_transmit(' ');
		UART0_transmit(' ');
		UART0_transmit(outch);
		UART0_transmit('O');	//여기에 있는 문자들을 컴퓨터로 송신 출력
	}
	else	//player2 순서이면
	{
		UART0_transmit('\t');	//공백을 컴퓨터로 송신 출력
	}
	UART0_transmit('\t');
	UART0_transmit(pt1ch);
	UART0_transmit('\n');
	UART0_transmit('\b');
	UART0_transmit(pt2ch);
	UART0_transmit('\r');
	UART0_transmit('P');
	UART0_transmit('2');
	if( playerset == 1 )	//player2 순서이면
	{
		UART0_transmit('\t');
		UART0_transmit(strikech);
		UART0_transmit('S');
		UART0_transmit(' ');
		UART0_transmit(' ');
		UART0_transmit(bch);
		UART0_transmit('B');
		UART0_transmit(' ');
		UART0_transmit(' ');
		UART0_transmit(outch);
		UART0_transmit('O');	//이들의 문자들을 컴퓨터로 송신 출력
	}
	UART0_transmit('\n');
	UART0_transmit('\n');
	UART0_transmit('\r');
}	//UI함수는 전체적으로 player1, 2의 볼, 스트라이크 카운터, 점수 등등 전광판처럼 보여주기위한 것이다
void pointset1(void)	//player1의 점수 계산 함수
{
	if(ball == 4)	//ball이 4이면
	{
		++pt1;	//점수 증가
		ball = 0;	
		strike = 0;	//볼, 스트라이크 초기화
	}
	if(strike == 3)	//strike가 3이면
	{
		++out;	//아웃 증가
		ball = 0;
		strike = 0;	//볼, 스트라이크 초기화
	}

	if(out == 3)	//out이 3이면
	{
		ball = 0;
		strike = 0;
		out = 0;	//전체 초기화
		playerset = 1;	//player2가 공격
		chcnt++;	//공수교대 횟수를 알수있음, 게임 회차에 영향 
		cli();	//전체 인터럽트 비활성
		TCCR0 |= (0 << CS02) | (0 << CS01) | (0 << CS00);	//타이머 카운터 정지
		TIMSK |= (0 << TOIE0);	//오버플로 인터럽트 비활성
		while( change != 128 )	//약간의 시간동안 반복
		{
			change++;	//change 증가

			PORTG = 0x00;
			PORTG |= 0x0f;
			PORTG &= 0x04;

			PORTC = 0x39;
			_delay_ms(5);

			PORTG |= 0x0f;
			PORTG &= 0x02;

			PORTC = 0x76;
			_delay_ms(5);	//세그먼트 중앙에 CH를 나타냄 (공수교대)
		}
		change = 0;	//change 초기화
		sei();	//전체 인터럽트 활성화
	}
}

void pointset2(void)	//player2의 점수 계산 함수
{
	if(ball == 4)	//ball이 4이면
	{
		++pt2;	//점수 증가
		ball = 0;
		strike = 0;	//볼, 스트라이크 초기화
	}
	if(strike == 3)	//strike가 3이면
	{
		++out;	//아웃 증가
		ball = 0;
		strike = 0;	//볼, 스트라이크 초기화
	}

	if(out == 3)	//out이 3이면
	{
		ball = 0;
		strike = 0;
		out = 0;	//전체 초기화
		playerset = 0;	//player1이 공격
		chcnt++;	//공수교대 횟수를 알수있음, 게임 회차에 영향 
		cli();	//전체 인터럽트 비활성
		TCCR0 |= (0 << CS02) | (0 << CS01) | (0 << CS00);	//타이머 카운터 정지
		TIMSK |= (0 << TOIE0);	//오버플로 인터럽트 비활성
		while( change != 128 )	//약간의 시간동안 반복
		{
			change++;	//change 증가

			PORTG = 0x00;
			PORTG |= 0x0f;
			PORTG &= 0x04;

			PORTC = 0x39;
			_delay_ms(5);

			PORTG |= 0x0f;
			PORTG &= 0x02;

			PORTC = 0x76;
			_delay_ms(5);	//세그먼트 중앙에 CH 표시 (공수교대)
		}
		change = 0;	//change 초기화
		sei();	//전체 인터럽트 활성화
	}
}

void point1(void) // player1의 점수
{
	PORTG |= 0x09;
	PORTG &= 0x08;

	PORTC = numbers[pt1];	//세그먼트 4번째 자리에 표시
}

void point2(void) // player2의 점수
{
	PORTG |= 0x09;
	PORTG &= 0x01;

	PORTC = numbers[pt2];	//세그먼트 1번째 자리에 표시
}

void victory(void) //승리 세그먼트 표시
{
	PORTG |= 0x0f;
	PORTG &= 0x01;

	PORTC  = 0x0f;
	_delay_ms(5);

	PORTG |= 0x0f;
	PORTG &= 0x08;

	PORTC  = 0x39;
	_delay_ms(5);

	PORTG |= 0x0f;
	PORTG &= 0x04;

	PORTC  = 0x73;
	_delay_ms(5);	//세그먼트에 [P?]표시 (? 제외)

	if( pt1 > pt2 )	//만약 player1의 점수가 높으면
	{
		PORTG |= 0x0f;
		PORTG &= 0x02;
	
		PORTC  = 0x06;	//세그먼트 2번째 자리에 1 표시( 위의 ?자리 )
	}
	else if( pt1 < pt2 )	//만약 player2의 점수가 높으면
	{
		PORTG |= 0x0f;
		PORTG &= 0x02;
	
		PORTC  = 0x5b;	//세그먼트 2번째 자리에 2 표시( 위의 ?자리 )
	}
	else	//그렇지 않으면 (무승부)
	{
		PORTG |= 0x0f;
		PORTG &= 0x02;
	
		PORTC  = 0x40;	//세그먼트 2번째 자리에 - 표시
	}	
	_delay_ms(5);
}

void ballseg(void)	//ball카운트를 세그먼트로 표현
{
	PORTG |= 0x0f;
	PORTG &= 0x04;
	
	PORTC = numbers[ball];
	_delay_ms(5);

	PORTG |= 0x0f;
	PORTG &= 0x02;
	
	PORTC = 0xff;
	_delay_ms(5);
}

void strikeseg(void)	//strike카운트를 세그먼트로 표현
{
	PORTG |= 0x0f;
	PORTG &= 0x04;
	
	PORTC = numbers[strike];
	_delay_ms(5);

	PORTG |= 0x0f;
	PORTG &= 0x02;
	
	PORTC = 0xed;
	_delay_ms(5);
}

void outseg(void)	//out카운트를 세그먼트로 표현
{
	PORTG |= 0x0f;
	PORTG &= 0x04;
	
	PORTC = numbers[out];
	_delay_ms(5);

	PORTG |= 0x0f;
	PORTG &= 0x02;
	
	PORTC = 0xbf;
	_delay_ms(5);
}

void hitball(void) //공을 쳤을 때 공이 날아가는 것을 표현
{
	char ballcntl = 0x02;	//LED에서 공을 표현하기 위함

	PORTA = 0x80;	//LED에 투수 표현
	while( ballcntl != 0 )	//공이 넘어갈 때까지 반복
	{
		ballcntl <<= 1;	//한비트씩 이동
		PORTA = ( ballcntl | 0x80 );	//LED에 투수는 제자리, 공만 움직임
		_delay_ms(count*5); //공이 랜덤한 속도로 날아감( 오버플로 인터럽트 영향 )
	}
	if(playerset == 0)	//player1이 쳤으면
	{
		pt1++;	//점수 증가
		ball = 0;
		strike = 0;	//볼, 스트라이크 초기화
	}
	else if(playerset == 1)	//player2가 쳤으면
	{
		pt2++;	//점수 증가
		ball = 0;
		strike = 0;	//볼, 스트라이크 증가
	}
}

void thrball(unsigned char speed) //ball을 던지는 함수 (코앞에서 떨어짐)
{
	int sp, sw = 0;
	char ballcntl = 0x80;	//공을 표현하기 위함
	hit = 0;

	if(speed == '0') { sp = 150; }
	else if(speed == '1') { sp = 100; }
	else if(speed == '2') { sp = 50; }
	else if(speed == '3') { sp = 25; }
	else { sp = 17; }	//키보드 0~4를 입력 받았을 때 공의 속도 설정
	while ( PORTA != 0x0a )	//코앞에서 공이 멈춤, 그 전까지 공이 날아감
	{
		ballcntl >>= 1;	//한 비트씩 공이 날아감
		PORTA = ballcntl | 0x02;	//LED에 공과 타자를 표현, 0x02는 타자를 나타냄
		_delay_ms(sp);	//공의 속도에 영향을 받음
		if( (hit == 1) && (sw == 0) )	//만약 sw1을 눌렀으면(sw는 공이 날아가는 도중에 계속 if문					이 반복됨을 방지)
		{
			sw = 1;	//if문이 한번만 실행 되도록 하기위함
			DDRE |= 0x10;	//포트E4 을 출력상태로 설정
			strike++;	//스트라이크 증가
		}
	}
	PORTA = 0x02;	//공이 코앞에서 없어지므로 타자만 남는 것을 표현
	_delay_ms(100);
	if( sw == 0 )	//sw1을 누르지 않았으면
	{
		ball++;	//볼 증가
	}
}

void thrstrike(unsigned char speed) //strike를 던지는 함수 (끝까지 날아감)
{
	int sp, bitcnt = 8, sw = 0;
	char ballcntl = 0x80;	//공을 표현하기 위함
	hit = 0;

	if(speed == '5') { sp = 150; }
	else if(speed == '6') { sp = 100; }
	else if(speed == '7') { sp = 50; }
	else if(speed == '8') { sp = 25; }
	else { sp = 17; }	//키보드 5~9를 입력받음, 그에 따른 속도
	while ( bitcnt-- )	//bitcnt = 8이므로 공이 비트 끝까지 날아감
	{
		ballcntl >>= 1;	//한비트씩 이동
		PORTA = ballcntl | 0x02;	//공과 타자를 표현, 0x02는 타자
		_delay_ms(sp);	//공의속도에 영향을 받음
		if( (hit == 1) && (sw == 0) )	//sw1을 누르면,	sw는 if문이 반복되지 않기 위함
		{
			sw = 1;	//if문이 반복되지 않도록함
			DDRE |= 0x10;	//포트E4를 출력상태로 설정
			if(PORTA == 0x02)	//누르는 순간 공이랑 비트가 겹치면( 공을 쳤다면 )
			{
				UART0_transmit('\a');	//컴퓨터로 송신( 컴퓨터에서 소리가남 )
				hitball();	//공이 날아감
				break;	//while문을 빠져나옴
			}
			else if( PORTA >= 0x04 )	//공을 치지 못하면( 공이 오는 중 )
			{
				sw = 2;	//else if문에 들어가지 않기 위함
				strike++;	//스트라이크 증가
			}
		}
		else if( (sw == 0) && (PORTA == 0x02) && (bitcnt <= 1) )	//공이 지나가고 sw1을 누르								지 않았으면
		{
			strike++;	//스트라이크 증가
		}
		else if( (sw == 1) && (PORTA == 0x02) && (bitcnt <= 1) )	//공이 지나가고 sw1을 눌렀								으면
		{
			strike++;	//스트라이크 증가
		}
	}
}
ISR(TIMER0_OVF_vect)	//오버플로 인터럽트 함수
{
	count++;	//count 증가

	if( state == 0 )	//state가 0이면
	{
		point1();
		_delay_ms(5);
		point2();
		_delay_ms(5);	//세그먼트에 점수표시
	}
	else if( state == 1 )	//state가 1이면
	{
		strikeseg();	//세그먼트에 스트라이크 표시
	}
	else if( state == 2 )	//2이면
	{
		ballseg();	//볼 표시
	}
	else if( state == 3 )	//3이면
	{
		outseg();	//아웃 표시
	}

	if(count == 10)	//count가 10이면
	{
		count = 1;	//1로 초기화, hitball 함수 내부에 영향
	}
}

ISR(INT4_vect) //sw1 인터럽트, 방망이 휘두르기
{
	hit = 1;	//눌렀다는 표시
	PORTA &= ~(0x02);
	_delay_ms(30);
	PORTA |= 0x02;	//누르는 순간 깜빡임
}

ISR(INT5_vect) //sw2 인터럽트, 각종 카운트를 세그먼트로 볼수 있음
{
	state++;	//누를때 마다 state 증가( 오버플로 인터럽트 함수 참고 )
	state %= 4; //0~3까지 표현
}

void INIT_INT4(void) //인터럽트 sw1 활성화, 하강에지 인터럽트 발생
{
	EIMSK |= (1 << INT4);
	EICRB |= (1 << ISC41);
}
void INIT_INT5(void) //인터럽트 sw2 활성화, 하강에지 인터럽트 발생
{
	EIMSK |= (1 << INT5);
	EICRB |= (1 << ISC51);
}

void UART0_init(void) // 유아트 설정
{
	UBRR0H = 0x00; // 9,600 보율로 설정
	UBRR0L = 207;
	UCSR0A |= _BV(U2X0); // 2배속 모드
	// 비동기, 8비트 데이터, 패리티 없음, 1비트 정지 비트 모드
	UCSR0C |= 0x06;
	UCSR0B |= _BV(RXEN0); // 송수신 가능
	UCSR0B |= _BV(TXEN0);
}

void UART0_transmit(char data) //유아트 송신
{
	while( !(UCSR0A & (1 << UDRE0)) ); // 송신 가능 대기
	UDR0 = data; // 데이터 전송
}

unsigned char UART0_receive(void) //유아트 수신
{
	while( !(UCSR0A & (1<<RXC0)) ); // 데이터 수신 대기
	return UDR0;
}

int main(void)
{
	char switon = 0;	//게임 시작을 위한 char 변수 선언
	UART0_init(); // UART0 초기화

	TCCR0 |= (1 << CS02) | (1 << CS01) | (1 << CS00);	//분주비 1024
	TIMSK |= (1 << TOIE0);	//오버플로 인터럽트 활성화

	INIT_INT4();	//인터럽트4 설정 함수
	INIT_INT5();	//인터럽트5 설정 함수
	sei();	//전체 인터럽트 활성화

	DDRA = 0xff;	//포트A핀 모두를 출력으로 설정

	DDRC = 0xff;	//포트C핀 모두를 출력으로 설정	(세그먼트 LED)
	DDRG = 0x0f;	//포트G핀 앞 4자리를 출력으로 설정	(세그먼트 4자리)

	while(switon != 's')	//switon이 s가 될 때까지 무한 반복
	{
		switon = UART0_receive();	//switon의 값은 컴퓨터 입력 값이다.( UART가 수신받은 데					이터 )
	}

	UI();	//UI함수, tera term에 출력을 위한 함수
	UIball();	//tera term에 몇 회차, 무슨 공을 던졌는지 나타내는 함수

	while(switon)	//위에서 s를 입력한 이상 무한 루프
	{
		PORTA = 0x82;	//포트A에 출력상태 10000010, 각각 타자와 투수를 나타냄
		DDRE = ~(0x30);	//sw1 sw2를 입력상태로 한다 (포트E4~5)
		
		TCCR0 |= (1 << CS02) | (1 << CS01) | (1 << CS00);	//분주비 1024
		TIMSK |= (1 << TOIE0);	//오버플로 인터럽트 활성화
		if(playerset == 0) //playerset 으로 순서 결정, player 1의 차례
		{
			ballch = UART0_receive();	//키보드 입력받음, ballch에 저장
			if( ('0' <= ballch) && (ballch <= '9') )
			{
				UART0_transmit(ballch);	//0~9일때만 컴퓨터로 송신
			}

			if( ('0' <= ballch) && (ballch <= '4') )	//0~4를 수신 받으면
			{
				thrball(ballch);	//ball을 던지는 함수
				hit = 0;	//hit를 0으로 초기화
			}
			else if( ('5'<= ballch) && (ballch <= '9') )	//5~9를 수신 받으면
			{
				thrstrike(ballch);	//strike를 던지는 함수
				hit = 0;	//hit를 0으로 초기화
			}
			pointset1();	//player1 점수 계산 함수
			if( (('0' <= ballch) && (ballch <= '9')) || (ballch == 's') )	//공을 던졌거나 									s를 입력 받으면
			{
				UI();	//컴퓨터로 전광판 출력 
				UIball();	//몇번 공을 던지는지, 게임회차 출력
			}
		}
		PORTA = 0x82;	//타자와 투수가 서있는 처음상태로 초기화
		if( chcnt == 6 || (( pt1 == 10) || (pt2 == 10 )) )	//경기가 3회말까지 진행됬거나, 10점인 							플레이어가 나오면
		{
			break;	//경기 끝
		}
		if(playerset == 1) //player 2의 차례
		{
			ballch = UART0_receive();	//키보드 입력받음, ballch에 저장
			if( ('0' <= ballch) && (ballch <= '9') )
			{
				UART0_transmit(ballch);	//0~9일때만 컴퓨터로 송신
			}
	
			if( ('0' <= ballch) && (ballch <= '4') )	//0~4를 수신 받으면
			{
				thrball(ballch);	//ball을 던지는 함수
				hit = 0;	//hit를 0으로 초기화
			}
			else if( ('5'<= ballch) && (ballch <= '9') )	//5~9를 수신 받으면
			{
				thrstrike(ballch);	//strike를 던지는 함수
				hit = 0;	//hit를 0으로 초기화
			}
			pointset2();	//player2 점수 계산 함수
			if( (('0' <= ballch) && (ballch <= '9')) || (ballch == 's') )	//공을 던졌거나 									s를 입력 받으면
			{
				UI();	//컴퓨터로 전광판 출력 
				UIball();	//몇번 공을 던지는지, 게임회차 출력
			}
		}
		if( chcnt == 6 || (( pt1 == 10) || (pt2 == 10 )) )	//경기가 3회말까지 진행됬거나, 10점인 							플레이어가 나오면
		{
			break;	//경기 끝
		}
	}
	UART0_transmit('F');
	UART0_transmit('i');
	UART0_transmit('n');
	UART0_transmit('i');
	UART0_transmit('s');
	UART0_transmit('h');	//컴퓨터로 Finish 출력
	cli();	//전체 인터럽트 비활성
	while(1)	//무한 루프
	{
		victory();	//승리한 플레이어 세그먼트 표시
		
		PORTA = 0x55;	//LED 01010101 표시
	}
	return 0;
}
/*이번 프로젝트에서 다양한 기능을 이용했다.
UART통신을 이용해 키보드로 입력 받아 게임을 시작하거나 공의 구속, 어떤 공을 던질지 결정하게 하고 사용자가 보기 편하게 컴퓨터로 송신하여 전광판을 만들어 보여주는 기능을 구현했다.
세그먼트는 잔상 효과를 이용해 원하는 자리에 원하는 LED가 나오도록 했다.
사용자가 보기 편하도록 전광판을 만들어, 스위치 2번을 누르면 인터럽트 발생, 그러면 세그먼트가 다른 카운트 상태를 보여주도록 했다.
나머지 LED와 스위치 1번은 각각 투수, 타자, 공과 방망이를 표현해 1번을 누르면 방망이를 휘두르는 기능을 적용했다.비트가 움직이는 도중에 방망이로 쳐야하기 때문에 인터럽트 기능을 이용하여 구현했다.
그리고 타이머 카운터를 이용해 세그먼트가 항상 표시 될 수 있도록 오버플로 인터럽트를 이용했고 상황에 맞게 인터럽트를 활성 / 비활성으로 만들어 게임 진행에 차질이 없도록 구현하였다.*/