# websocket-client-in-c
a simple websocket client in c using only sockets
## What is websocket?
   The WebSocket Protocol enables two-way communication between a client and a server,
   The protocol consists of an opening handshake followed by basic message framing, layered over TCP.  
   The goal of this technology is to provide a mechanism for browser-based applications that need two-way communication with servers that does not rely on opening multiple HTTP connections
## The Handshake 
whenever a client connect to a websocket server(a nodejs server in our case here) it sends an http header to request an upgrade from the HTTP protocol to the websocket protocol, as following:
```
GET / HTTP/1.1
Host: 127.0.0.1:8828
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: 1ZRkVeZ4mQIMphh+3ezlSQ==
Sec-WebSocket-Version: 13
```
When the server accepts the upgrade request it sends an HTTP header as following to the client showing that the next messages are going to be over websocket:
```
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: ENOeuWlWBWN/kouv3Z6SXkIfFMI=
```
Now our handshake is successfull.
You can notice ```Sec-WebSocket-Key``` and ```Sec-WebSocket-Accept``` are a base64 encoded but for the sake of simplicity we are going to keep them constant in out code because they don't play a crucial role in the websocket communications and the same goes for masking(explained later)
## websocket framing protocol 
Now that the handshake is successful everything we send or recieve is over the websocket, but we can't send raw bytes(data) to the server we have to follow some basic protocol framing as described in [RFC6455#28](https://datatracker.ietf.org/doc/html/rfc6455#page-28)
```
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+

```
the first 4 bits ```0 1 2 3``` the first one is FIN which indicates that this is the final fragment in a message by default we always set it to 1 the same for RSV1, RSV2, RSV3 are set to 0 ```1000``` the next 4 bits are set to ```0001``` by us which means this is a text frame so basically the first byte(8 bits) is constant ```1000 0001``` = 0x81(in hex).
the next bit is set for the mask, the client is always obliged to mask the payload he sends to the server while the server doesn't bother to mask anything(masking is not related to security it literally does what it says 'masking') [Read More: RFC6455](https://datatracker.ietf.org/doc/html/rfc6455).


the next 7 bits are set for the payload size if it is under 126(because the max binary in 7b is 1111111 which is 127 but we use 1111110(126) as an indicator to reserver the next 2 bytes for the size) 
if our payload size is over or equal 126 then we have to reserver the next 2 bytes for the size, 2 bytes can support the size of 65535 if its full then we set the 126 to 127 instead and reserver an extra 6 bytes for the size.
## Masking
Now in the frame we add the 4 bytes of the masking key in out code the masking key is constant ```unsigned char mask_key[]= {0x31,0xee,0xd5,0x76};```.
The masking algorithm is a simple XOR, iterate through the payload i-times and for each byte XOR it with ```mask_key[i mod 4]```
