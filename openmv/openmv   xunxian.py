THRESHOLD = (0, 64)                             #黑线阈值
BINARY_VISIBLE = True
import sensor, image, time,math
from machine import UART,Timer
from machine import UART
import time,pyb
from pyb import UART,Timer
import ustruct
from image import SEARCH_EX, SEARCH_DS
led = pyb.LED(4)
global f
f=0
def tick(timer):
    global f
    f=f+1
tim = Timer(2, freq=1000)      # 使用定时器2创建定时器对象-以1Hz触发
tim.callback(tick)
#串口初始化
flag=0 #转弯标志位
uart = UART(3, 57600)
uart.init(57600, bits=8, parity=None, stop=1)#d4和d5引脚
a=0
b=0
def sending_data(c,flag):
    global uart;
    #frame=[0x2C,18,cx%0xff,int(cx/0xff),cy%0xff,int(cy/0xff),0x5B];
    #data = bytearray(frame)
    data = ustruct.pack("<bbhhhhb",           #格式为俩个字符俩个短整型(2字节)
                   0x2C,                      #帧头1
                   0x12,                      #帧头2
                   int(c),                    # up sample by 4    #数据1
                   int(flag),                 # up sample by 4    #数据2
                   0x5B)
    uart.write(data);                         #必须要传入一个字节数组

time.sleep_ms(100)
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()
time.sleep_ms(100)

while(True):
    led.on()
    global d
    clock.tick()
    img = sensor.snapshot().replace(vflip=True,
                                    hmirror=False,
                                    transpose=True).lens_corr(strength = 1.8, zoom = 1.0)
    img.mean(1)       #均值滤波
    img.binary([THRESHOLD])#灰度二值化函数
    img.dilate(2)
    line = img.get_regression([(64,64)], robust = True)
    if (line):
        b = abs(line.rho())-img.width()/2+14
        if line.theta()>90:
            a = line.theta()-180
        else:
            a = line.theta()
        img.draw_line(line.line(), color = 127)
        #print(b,line.magnitude(),b)
    line = img.get_regression([(255,255)if BINARY_VISIBLE else THRESHOLD])
    if (line):
        img.draw_line(line.line(), color = 127)
        b = abs(line.rho())-img.width()/2+14
        if line.theta()>90:
            a = line.theta()-180
        else:
            a = line.theta()
        img.draw_line(line.line(), color = 127)
        line = img.get_regression([(255,255) if BINARY_VISIBLE else THRESHOLD])
    print(a,b)
    if((a>=-90 and a<=90)and (b>=-40 and b<=20)):
        if a>22:
            a=22
        if a<-25:
            a=-25
        a+=100
        if flag>0:
            d+=1
        if f>=25:
            f=0
            sending_data(a,flag)
    elif a>=0 and b<-40:
        if flag>0:
            d+=1
        if f>=25:
            f=0
            sending_data(-b*0.32+100,flag)
    elif b>0 and a>7:
        x=0.5*b
        if x>22:
            x=22
        if x<-25:
            x=-25
        if flag>0:
            d+=1
        if f>=25:
            f=0
            sending_data(x+100,flag)
    elif a<0 and b<0 :
        x=0.5*b
        if x>22:
            x=22
        if x<-25:
            x=-25
        if flag>0:
            d+=1
        if f>=25:
            f=0
            sending_data(x+100,flag)
    else:
        y=-0.5*b
        if y<-25:
            y=-25
        if y>22:
            y=22
        if flag>0:
            d+=1
        if f>=25:
            f=0
            sending_data(y+100,flag)
    #print(clock.fps())
