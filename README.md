Lab for the course "Distribuited Programming 1"


tips: for build
gcc -g -Wall -DTRACE -o client_test client_test.c errlib.c sockwrap.c

to cpimpile server 2.3
gcc -g -Wall -DTRACE -o server server1_main.c ../errlib.c ../sockwrap.c
