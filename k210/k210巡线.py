THRESHOLD = (0,64)
BINARY_VISIBLE = True
import sensor, image, time
from machine import UART,Timer
from image import SEARCH_EX, SEARCH_DS
import KPU as kpu
import lcd
import ustruct
from fpioa_manager import fm
fm.register(9, fm.fpioa.UART1_TX, force=True)
fm.register(10, fm.fpioa.UART1_RX, force=True)
uart_A = UART(UART.UART1, 57600, 8, 0, 1, timeout=1000, read_buf_len=4096)
import time
time.sleep_ms(100)
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)
clock = time.clock()
a=0
b=0
flag=0
#==============================================数据发送函数================================================#
def sending_data(c,flag):
    global uart;
    data = ustruct.pack("<bbhhb",           #格式为俩个字符俩个短整型(2字节)
                   0x2C,                      #帧头1
                   0x12,                      #帧头2
                   int(c),                    # up sample by 4    #数据1
                   int(flag),                 # up sample by 4    #数据2
                   0x5B)
    uart_A.write(data);                         #必须要传入一个字节数组

time.sleep_ms(100)
while(True):
    clock.tick()

    img = sensor.snapshot().replace(vflip=True,
                                    hmirror=False,
                                    transpose=True).lens_corr(strength = 1.8, zoom = 1.0)#视角旋转90
    img.mean(1) #均值滤波
    img.binary([THRESHOLD])#灰度二值化
    line = img.get_regression([(60,60)], robust = True)
    if (line):
        b = abs(line.rho())-img.width()/2-8
        if line.theta()>90:
            a = line.theta()-180
        else:
            a = line.theta()
        img.draw_line(line.line(), color = 127)
        #print(b,line.magnitude(),b)
    line = img.get_regression([(255,255) if BINARY_VISIBLE else THRESHOLD])
    if (line):
        #print(line.line())
        img.draw_line(line.line(), color = 127)
        b = abs(line.rho())-img.width()/2-8
        if line.theta()>90:
            a = line.theta()-180
        else:
            a = line.theta()
        img.draw_line(line.line(), color = 127)
        #print(b)
        line = img.get_regression([(255,255) if BINARY_VISIBLE else THRESHOLD])
    lcd.display(img)
    lcd.draw_string(60, 100, "reo:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 100, str(a), lcd.RED, lcd.BLACK)
    lcd.draw_string(30, 150, "skewing:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 150, str(b), lcd.RED, lcd.BLACK)
    lcd.draw_string(50, 200, "flag:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 200, str(flag), lcd.RED, lcd.BLACK)
    #print(a,b)
    if((a>=-60 and a<=40)and (b>=-30 and b<=44)):
        if a>22:
            a=22
        if a<-25:
            a=-25
        a+=100
        sending_data(a,flag)
    elif a>=-1 and b>44:
        sending_data(b*0.32+100,flag)
    elif a<0 and b<0 :
        x=0.5*b
        if x>22:
            x=22
        if x<-25:
            x=-25
        sending_data(x+100,flag)
    else:
        y=0.5*b
        if y<-25:
            y=-25
        if y>22:
            y=22
        sending_data(y+100,flag)
