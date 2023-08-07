#include <stdio.h>//printf
#include <string.h>//strlen, strcpy,memcpy
#include <netdb.h>
#include <stdlib.h>//malloc, free, exit
#include <sys/socket.h>
#include <math.h>

#include <unistd.h>//sleep(),close()



unsigned short port=8828;       
char *ipaddr = "127.0.0.1";


// Mask message with a simple XOR algorithm
char *mask_char(unsigned char msg[],unsigned char mask_key[],size_t message_size){
    unsigned char *xord= malloc(message_size);
    for (int i = 0; i <message_size; ++i)
    {
        xord[i] = msg[i] ^ mask_key[i % 4];
    }
        return xord;
}
//convert decimals to hex
char *b4_decimal_to_hex(unsigned long n){
    if (n <65535)
    {
        unsigned char *bytes=malloc(2);
        bytes[0] = (n >> 8) & 0xFF;
        bytes[1] = n & 0xFF;
        return bytes;
    }
    unsigned char *bytes=malloc(4);
    bytes[0] = (n >> 24) & 0xFF;
    bytes[1] = (n >> 16) & 0xFF;
    bytes[2] = (n >> 8) & 0xFF;
    bytes[3] = n & 0xFF;
    return bytes;
}

int msg_bytes_len(int msg_len){
    if (msg_len<128)
    {
        return 1;
    }else if (msg_len <65535)
    {
        return 3;
}
}
// calculating the message size 

char *message_size_frame(int msglen){
    if (msglen<126)
    {

        unsigned char *p_size_bytes=malloc(1);
        p_size_bytes[0] = msglen^0b10000000;
        return p_size_bytes;
    
    }else if(msglen  <65535+ 126 ){//65535(1111 1111 1111 1111) + 126(1111110)
        unsigned char *p_size_bytes=malloc(3);
        p_size_bytes[0] = 0b11111110;//126
        char *arr=b4_decimal_to_hex(msglen);
        p_size_bytes[1]=arr[0];
        p_size_bytes[2]=arr[1];

        free(arr);
        
        return p_size_bytes;
    
    }
}

// building the frame in the correct format 
char *wsframe_build(char msg[], char mask_key[],size_t message_size){
    unsigned char init_head=0x81;//first byte of the websocket frame protocol 10000001,FIN=1,OPCODE=0001(text)
    unsigned char *masked_message = mask_char(msg,mask_key,message_size); //masking the message using a spesific masking key
    unsigned char *message_size_frame_bytes= message_size_frame(message_size);//the size of the frame as a header bytes
    int message_size_frame_bytes_lenght = msg_bytes_len(message_size);//how many bytes reserved for the message size
    unsigned char *frame = malloc(message_size+100);
    frame[0] = init_head;
    int index = 1;
    //adding message lenght frame
    memcpy(frame+index,message_size_frame_bytes,message_size_frame_bytes_lenght);
    index = index+ message_size_frame_bytes_lenght;
    //adding mask 4 bytes key
    memcpy(frame+index,mask_key,4);
    index = index+4;
    //adding masked message frame
    memcpy(frame+index,masked_message,message_size);
    index = index+message_size;

    free(message_size_frame_bytes);
    free(masked_message);

    return frame;
}

void send_socket_frame(int s,char *message, size_t message_size){
    unsigned char mask_key[]= {0x31,0xee,0xd5,0x76};// masking key 
    int __size_full_frame=message_size+4+1+msg_bytes_len(message_size);
    unsigned char *frame = wsframe_build(message,mask_key,message_size);
    if (send(s, frame, __size_full_frame, 0) < 0)
        {
            printf("Send()");
            exit(5);
        }
    
    free(frame);

}
//used for testing
char* repeat (char c , int count )
{
    char *cc = malloc(count);
     for (int i = 0; i<count;i++){
        cc[i]  = c;
    }
    return cc;
}
void send_socket(int s, char *buffer, size_t buffer_size){
    if (send(s, buffer, strlen(buffer), 0) < 0)
        {
            printf("Send()");
            exit(5);
        }
}

void recieve_ws_frame(int s){
    unsigned char first_two_bytes[2];
    recv(s,first_two_bytes,2,0);
    if (first_two_bytes[1] == 0x7e) // if the size byte == 01111110, means the reserved bytes for size is full and the next two bytes contain the size
    {
        unsigned char extra_bytes[2];
        recv(s,extra_bytes,2,0);
        size_t frame_size = extra_bytes[0]<<8 | extra_bytes[1];
        unsigned char frame[frame_size];
        int recieved_size = recv(s,frame,frame_size,0);
        frame[frame_size] = '\0';
        printf("Recieved frame size: %d\n",recieved_size);
        int index=0;
        printf("Frame: %s\n",frame );
    }else{
        size_t frsize= first_two_bytes[1];
        unsigned char frame[frsize];
        int recieved_size = recv(s,frame,frsize,0);
        printf("Recieved frame size: :%d\n",recieved_size);
        printf("%s\n",frame );
    }
}
void ping_pong(s){
    char junk[100];
    char recvd_char[10];
    char pong[4];
    send_socket_frame(s,"ping",4);
    int rv;
    rv = recv(s,junk,sizeof(junk),0);
    rv = recv(s,recvd_char,sizeof(recvd_char),0);
    memcpy(pong,recvd_char+2,sizeof(recvd_char));
    if (strcmp(pong,"pong")==0)
    {
    printf(" + Websocket communication established\n");
    }else{
        printf(" - Couldn't recieve 'pong' message from the ws server\n");
        close(s);
        exit(1);
    }
}
int main(){

    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s;                     
    hostnm = gethostbyname(ipaddr);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    char *handshake_request = "GET / HTTP/1.1\r\n"
                             "Host: %s:%d\r\n"
                             "Upgrade: websocket\r\n"
                             "Connection: Upgrade\r\n"
                             "Sec-WebSocket-Key: 1ZRkVeZ4mQIMphh+3ezlSQ==\r\n"
                             "Sec-WebSocket-Version: 13\r\n\r\n";

    char buf[1024];
    snprintf(buf, sizeof(buf), handshake_request, ipaddr, port);

    server.sin_family      = AF_INET;
    server.sin_port        = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket()");
        exit(3);
    }
    printf(" + Socket descriptor initialized\n");
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf(" - Couldn't connect to %s:%d\n",ipaddr,port);
        exit(4);
    }
    printf(" + Socket connected on %s:%d\n",ipaddr,port);

    send_socket(s,buf,strlen(buf));

    printf(" HTTP -> WS Upgrade request sent successfuly\n");
        if (recv(s, buf, sizeof(buf), 0) < 0)
        {
            printf(" - Couldn't recieve ");
            exit(6);
        }

        ping_pong(s);
   while(1){
        char *arr = repeat('A',200);
        send_socket_frame(s,arr,200);
        free(arr);
        sleep(1);//make sure you stay awake
        recieve_ws_frame(s);
    }
    close(s);

    printf("End\n");
    exit(0);

}
