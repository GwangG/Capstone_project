#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <limits.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include "/usr/include/mysql/mysql.h"
#define CALL 'A'

#define BUFF_SIZE   1024
#define MAX_BUILDING 2 //서버에서 가동할 수 있는 최대 빌딩의 수
#define FLOOR 20
#define NUM_ELEVATORS 6
#define MAX_PEOPLE 15 // 엘리베이터 정원
#define MAX_TOTAL 150 // 점검 받아야하는 수

/* 요청 구조체 */
typedef struct _REQUEST
{
    int start_floor; //현재층(요청이 이루어지는)
    int dest_floor;  //목적층
    int num_people;  //몇 명이 타는지
} Request;

typedef struct _FLOORNODE
{
    struct _FLOORNODE *prev;
    struct _FLOORNODE *next;
    int floor;  // -1 : 점검
    int people; //+ 태운다, - 내린다
} F_node;

typedef struct _REQUESTNODE
{
    struct _REQUESTNODE *prev;
    struct _REQUESTNODE *next;
    Request req;
} R_node;

typedef struct _FLOORLIST
{
    F_node *head;
    F_node *tail;
} F_list;
// 해당엘리베이터에 맞는 연결 리스트를 만든다. 이 리스트를 통해서 최고 마지막 도착지를 알 수 있다.

typedef struct _REQUESTLIST
{
    R_node *head;
    R_node *tail;
} R_list;

/* 엘리베이터 구조체 */
typedef struct _ELEVATOR
{
    int current_floor;
    int next_dest;
    int current_people;
    int total_people;
    int fix;
    int fix_time;
    F_list pending;
} Elevator;

typedef struct _INPUT
{
    char *mode;
    int *req_elevator_id;
    int *req_current_floor;
    int *req_dest_floor;
    int *req_num_people;
} Input;

typedef struct _SIMUL
{
    Elevator **elevators;
    Input *input;
} Simul; // 시뮬레이션 구조체

///////////////////////////////////////////////////////////////////////
void *input_f(void *data);
void *simul_f(void *data);
int DBconector(int id);
void socket_server();
void get_request(Input *input);
oid move_elevator(Elevator *elevators[6]);
int find_time(F_list list, F_node *target, int start, int end);
void init(Input **input, Simul **simul, Elevator *elevators[6]);
void insert_into_queue(int current_floor, int dest_floor, int num_people);
void R_list_insert(R_list list, int current_floor, int dest_floor, int num_people);
//////////////////////////////////////////////////////////////////////

R_list reqs;
int flag = 0;
///////////////////////////////////////////////////////////////////////
int main()
{
	Input *input;
	Simul *simul;
	Elevator *elevators[6];

	pthread_t input_thr;
  	pthread_t socket_thr;
  	pthread_t simul_thr[MAX_BUILDING];
  	int tid_input;
  	int tid_simul[MAX_BUILDING];
  	//원래는 소켓통신으로 할 예정이지만 현재는 알고리즘이 잘 돌아가는지 볼것이므로 scanf를 받아서 실행한다.


	init(&input, &simul,elevators);
    
    tid_input = pthread_create(&input_thr, NULL, input_f, (void *)input);
    if (tid_input != 0){
        perror("thread creation error: ");
        exit(0);
    }

    for(int i = 0 ; i < MAX_BUILDING;i++){
    	//현재 등록된 건물의 수만큼 thread를 생성한다.

    	tid_simul[i] = pthread_create(&simul_thr[i], NULL, simul_f, (void *)simul);
    	if (tid_simul[i] != 0){
       	perror("thread creation error: ");
       	exit(0);
   		}


    }
	//get_request(input);
	//DBconector(input->building_id);
	//insert_into_queue(*simul->input->req_current_floor, *simul->input->req_dest_floor, *simul->input->req_num_people);
    pthread_join(input_thr, NULL);

    pthread_join(simul_thr, NULL);

    return 0;
}

Elevator *LOOK(Elevator *elevators[6], Request *current){

	ideal = (F_node **)malloc(sizeof(F_node *) * size);
 	time_required = (int *)malloc(sizeof(int) * size);

 	for(int i = 0 ; i < NUM_ELEVATORS;i++){

 		ideal[i] = find_ideal_location(elevators[i], current->start_floor, current->dest_floor, current->start_floor);
  		time_required[i] = find_time(elevators[i]->pending, ideal[i], elevators[i]->current_floor, current->start_floor);
  		printf("%d번째 엘리베이터 소요시간: %d초 \n", i + 1, time_required[i]);

 	}
 	ideal_index = find_min(time_required, size);
  	free(time_required);
  	free(ideal);

    printf("엘리베이터 %d 호출에 응답 \n", ideal_index + 1);
    return elevators[ideal_index ];


}

Elevator *C_SCAN(Elevator *elevators[6], Request *current){



}

void *simul_f(void *data){
	int i;
    Simul *simul = (Simul *)data;
    Elevator *response; // 요청에 응답하는 엘리베이터
    F_node *location;   // 요청이 들어가는 위치
    Request current;    // 처리할 요청

    while(1){
    	if (*simul->input->mode == CALL)
        {
            get_request(simul->input);
        }
        insert_into_queue(*simul->input->req_current_floor, *simul->input->req_dest_floor, *simul->input->req_num_people);

        if (R_list_size(reqs) != 0)
        {
            current = *R_list_remove(reqs);
            response = LOOK(simul->elevators, &current);
            // 요청에 응답하는 엘리베이터에 정보 추가하기

            // 사람 태울 층 추가하기
            location = find_ideal_location(response, current.start_floor, current.dest_floor, current.start_floor);
            F_list_insert(response->pending, location, current.start_floor, current.num_people);
            //printf("@@@@@@@@@@@@@@@@@@@@@location : %d\n\n@@@@@@@@@@@@@@@@@@", location->floor);
            //printf("@@@@@@@@@@@@@@@@@@@@@location : %d\n\n@@@@@@@@@@@@@@@@@@", location->people);
            // 사람 내릴 층 추가하기
            location = find_ideal_location(response, current.start_floor, current.dest_floor, current.dest_floor);
            F_list_insert(response->pending, location, current.dest_floor, current.num_people * -1);
        }

        // 엘리베이터 이동시키기
        move_elevator(simul->elevators);

        sleep(1);


    }
}
void *input_f(void *data){

	Input *input = (Input *)data;
	while(1){
		if (*input->mode == CALL){
		}
		else{	

			scanf(" %c", input->mode);
		}
	}
}
/*
int DBconector(int id){
	//나중에 사용할껀데 일단은 로컬에서 테스트 할꺼니깐 주석처리

	MYSQL *conn;
 	MYSQL_RES *res;
 	MYSQL_ROW row;

 	char *server = "localhost";
 	char *user = "root";
 	char *password = "root";
 	char *database = "capstone";

 	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){
 		exit(1);
  	}
   if(mysql_query(conn,"show tables")){

   		exit(1);
   }

   res = mysql_use_result(conn);
  	printf("MYSQL Tables in mysql database : ");
  	while((row = mysql_fetch_row(res)) != NULL)
  		printf("%s \n",row[0]);


  	if(mysql_query(conn,"SELECT * FROM user"))
  	{
  	        return 1;
  	}

  	res = mysql_use_result(conn);

   	printf("Returning List of Names : \n");
   	while((row = mysql_fetch_row(res)) != NULL)
		printf("%s \n",row[0]);

   mysql_free_result(res);
   mysql_close(conn);

   printf("MYSQL RETURN : %d\n",row[0]);
   return row[0];

}
*/

void move_elevator(Elevator *elevators[6])
{
    int i;
    int to_ride = 0; // 태워야 할 사람 수
    int available;   // 정원이 초과될 시 최대로 태울수 있는 사람 수
    int leftover;    // 못 타고 남아있는 사람 수
    F_node *next_floor;
    F_node *pair;

    // 1. 다음 목적지를 구한다(있으면).
    // 1-1. 수리 요청인 경우 수리에 들어간다.
    // 2. 현재 층과 비교하여,
    // 3. 높으면 현재층 증가, 낮으면 감소, 같으면 사람을 태운다.
    // 4. 사람을 다 못 태우면 최대 수용 가능 인원만 태운다.
    // 4-1. 다 못 타고 남은 인원은 다시 엘리베이터를 호출한다.

    for (i = 0; i < NUM_ELEVATORS; i++)
    {
        if (elevators[i]->fix)
        {
            //fix_elevator(elevators[i]);
            continue;
        }

        if (F_list_size(elevators[i]->pending) > 0)
        {
            next_floor = F_list_peek(elevators[i]->pending);
            if (next_floor->floor == -1)
            {
                elevators[i]->fix = 1;
                F_list_remove(elevators[i]->pending);
            }
            else
            {
                if (elevators[i]->next_dest != next_floor->floor)
                {
                    elevators[i]->next_dest = next_floor->floor;
                }

                if (elevators[i]->next_dest > elevators[i]->current_floor)
                {
                    (elevators[i]->current_floor)++;
                }
                else if (elevators[i]->next_dest < elevators[i]->current_floor)
                {
                    (elevators[i]->current_floor)--;
                }
                else
                {
                    if (elevators[i]->current_floor == next_floor->floor)
                    {
                        available = MAX_PEOPLE - elevators[i]->current_people;
                        if (next_floor->people <= available)
                        {
                            elevators[i]->current_people += next_floor->people;
                            if (next_floor->people > 0)
                            {
                                elevators[i]->total_people += next_floor->people;
                            }
                            F_list_remove(elevators[i]->pending);
                        }
                        else
                        {
                            elevators[i]->current_people += available;
                            elevators[i]->total_people += available;
                            leftover = next_floor->people - available;
                            pair = next_floor->next;
                            while (1)
                            {
                                if (next_floor->people + pair->people == 0)
                                {
                                    break;
                                }
                                pair = pair->next;
                            }
                            F_list_remove(elevators[i]->pending);

                            pair->people = available * -1;

                            flag = 1;
                            insert_into_queue(elevators[i]->current_floor, pair->floor, leftover);
                        }
                    }
                }
            }
        }
    }
}

void socket_server(){
  //여기서 php랑 통신을 통해 파라미터를 받는다.
  int   client_socket;

  struct sockaddr_in   server_addr;

  char   buff[BUFF_SIZE+5];

  int building_id;
  int req_current_floor;
  int req_dest_floor;
  int req_num_people;

  client_socket  = socket( PF_INET, SOCK_STREAM, 0);
	if( -1 == client_socket){

		printf( "socket creat fail\n");
		exit( 1);
	}

	memset( &server_addr, 0, sizeof( server_addr));
	server_addr.sin_family     = AF_INET;
	server_addr.sin_port       = htons( 60000);
	server_addr.sin_addr.s_addr= inet_addr( "127.0.0.1");

	if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ){

		printf( "connect fail\n");
		exit( 1);
	}

	//write( client_socket, argv[1], strlen( argv[1])+1);
	read ( client_socket, buff, BUFF_SIZE);
	printf( "PHP BUFFER == %s\n", buff);
	close( client_socket);


}



void init(Input **input, Simul **simul, Elevator *elevators[6])
{
    int i, j;
    reqs.head = (R_node *)malloc(sizeof(R_node));
    reqs.tail = (R_node *)malloc(sizeof(R_node));
    reqs.head->prev = NULL;
    reqs.head->next = reqs.tail;
    reqs.tail->prev = reqs.head;
    reqs.tail->next = NULL;

    *input = (Input *)malloc(sizeof(Input));
    (*input)->mode = (char *)malloc(sizeof(char));
    (*input)->req_elevator_id = (int *)malloc(sizeof(int));
    (*input)->req_current_floor = (int *)malloc(sizeof(int));
    (*input)->req_dest_floor = (int *)malloc(sizeof(int));
    (*input)->req_num_people = (int *)malloc(sizeof(int));

    *simul = (Simul *)malloc(sizeof(Simul));
    (*simul)->input = *input;

    for (i = 0; i < NUM_ELEVATORS; i++)
    {
        elevators[i] = (Elevator *)malloc(sizeof(Elevator));
    }

    //엘리베이터 : 1, 2 - 저층, 3, 4 - 전층, 5, 6 - 고층
    for (i = 0; i < NUM_ELEVATORS; i++)
    {
        elevators[i]->pending.head = (F_node *)malloc(sizeof(F_node));
        elevators[i]->pending.tail = (F_node *)malloc(sizeof(F_node));
        elevators[i]->pending.head->floor = 0;
        elevators[i]->pending.tail->floor = 0;
        elevators[i]->pending.head->prev = NULL;
        elevators[i]->pending.head->next = elevators[i]->pending.tail;
        elevators[i]->pending.tail->prev = elevators[i]->pending.head;
        elevators[i]->pending.tail->next = NULL;

        elevators[i]->current_floor = 1;
        elevators[i]->next_dest = 1;
        elevators[i]->current_people = 0;
        elevators[i]->total_people = 0;
        elevators[i]->fix = 0;
        elevators[i]->fix_time = 0;
    }

    // 고층 엘리베이터는 처음 11층에 멈춰있음
    /*
    elevators[4]->current_floor = 11;
    elevators[4]->next_dest = 11;
    elevators[5]->current_floor = 11;
    elevators[5]->next_dest = 11;

    (*simul)->elevators = elevators;
	*/
}

int find_time(F_list list, F_node *target, int start, int end)
{
    int time = 0;
    F_node *curr = list.head->next;

    if(curr == target)
    {
        time += abs(end - start);
        return time;
    }

    if (curr->next == NULL)
    {
        time += abs(end - start);
        return time;
    }

    time += abs(curr->floor - start);
    time++;

    while (curr->next != target)
    {
        time += abs(curr->next->floor - curr->floor);
        time++;
        curr = curr->next;
    }
    time += abs(end - curr->floor);
    return time;
}

void get_request(Input *input)
{
    while (1)
    {
        printf("엘리베이터 아이디, 현재 층, 목적 층, 몇 명이 타는지 입력하시오 : ");
        fflush(stdout);
        scanf("%d %d %d %d",input->req_elevator_id, input->req_current_floor, input->req_dest_floor, input->req_num_people);
        printf("%d\n",*input->req_elevator_id);
        tcflush(0, TCIFLUSH);
        *input->mode = 0;
        flag = 1;
        break;
    }
}

void insert_into_queue(int current_floor, int dest_floor, int num_people)
{
    if (flag == 0)
    {
        return;
    }

    if (current_floor == dest_floor)
    {
        return;
    }

    if (current_floor > FLOOR || current_floor < 1 || dest_floor > FLOOR || dest_floor < 1)
    {
        return;
    }

    R_list_insert(reqs, current_floor, dest_floor, num_people);

    flag = 0;
}

void R_list_insert(R_list list, int current_floor, int dest_floor, int num_people)
{
	printf("R_list_insert(%d %d %d)\n", current_floor,dest_floor,num_people);
    R_node *new_node = (R_node *)malloc(sizeof(R_node));
    new_node->next = list.tail;
    new_node->prev = new_node->next->prev;
    new_node->prev->next = new_node;
    new_node->next->prev = new_node;
    new_node->req.start_floor = current_floor;
    new_node->req.dest_floor = dest_floor;
    new_node->req.num_people = num_people;
}

void F_list_insert(F_list list, F_node *after, int floor, int people)
{
    F_node *new_node = (F_node *)malloc(sizeof(F_node));
    new_node->next = after;
    new_node->prev = new_node->next->prev;
    new_node->next->prev = new_node;
    new_node->prev->next = new_node;
    new_node->floor = floor;
    new_node->people = people;
}

int F_list_size(F_list list)
{
    F_node *curr = list.head->next;
    int num = 0;

    while (curr != list.tail)
    {
        num++;
        curr = curr->next;
    }

    return num;
}

void F_list_remove(F_list list)
{
    F_node *to_remove = list.head->next;

    to_remove->prev->next = to_remove->next;
    to_remove->next->prev = to_remove->prev;
    to_remove->next = NULL;
    to_remove->prev = NULL;

    free(to_remove);
}

F_node *F_list_peek(F_list list)
{
    return list.head->next;
}

void print_F_list(F_list list)
{
    F_node *curr = list.head->next;
    while (curr != list.tail)
    {
        printf("(%dF %d명) ", curr->floor, curr->people);
        curr = curr->next;
    }
}
