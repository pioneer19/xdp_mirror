This project based on sdp tutorial 
https://github.com/xdp-project/xdp-tutorial/tree/master/packet03-redirecting

### HowToUse
build XDP program
```
make
```
this will produce `xdp_prog_mirror.o` which is XDP program.

mount bpf (probably needed only for statistics)
```
sudo mount -t bpf bpf /sys/fs/bpf/
```

load program for network interface
```
ip link set dev enp1s0f0 xdpgeneric obj xdp_prog_mirror.o sec xdp_udp_echo
# to unload
# ip link set dev enp1s0f0 xdpgeneric off
```
or using helper from https://github.com/xdp-project/xdp-tutorial project.
```
sudo ./xdp_loader    -d enp1s0f0 -F -- progsec xdp_icmp_echo
```
чтобы посмотреть статистику XDP по интерфейсу придется собрать helper из проекта https://github.com/xdp-project/xdp-tutorial
```
sudo ./xdp_stats -d enp1s0f0
```

Теперь нужно направить udp трафик на этот IP и XDP отправит его обратно, поменяв MAC и IP адреса, а также порты.
Для проверки можно отправить UDP пакет (с другой машины) с помощью socat и увидеть ответ в tcpdump. Открывать порт UPD
не надо - XDP будет отправлять обратно все UPD пакеты, приходящие на интерфейс, причем они даже не попадут в ядро, а
будут отправлены в сетевую карту сразу после считывания с нее в статический буфер драйвера карты.
```
echo test | socat - 'udp4:10.128.1.1:2000'
```
