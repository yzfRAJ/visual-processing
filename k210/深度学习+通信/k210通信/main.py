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
flag=0
global f
f=0
def tick(timer):
    global f
    f=f+1000


tim = Timer(Timer.TIMER0, Timer.CHANNEL0, mode=Timer.MODE_PERIODIC, period=1000, callback=tick)
# 使用定时器2创建定时器对象-以1Hz触发
templates1 = ["/sd/1.pgm"]#模板匹配转弯
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
        img.binary([THRESHOLD])#灰度二值化
    for t in templates1:     #如果与模板匹配
            template = image.Image(t) #template获取图片
            b = img.find_template(template, 0.70, step=4, search=SEARCH_EX)
            if b: #如果有目标
                if f>3000:
                    flag+=1
                    f=0
                img.draw_rectangle(b) #画矩形，框出匹配的目标
            print("模板匹配部分:%d" % a)
    lcd.display(img)
    lcd.draw_string(60, 100, "reo:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 100, str(a), lcd.RED, lcd.BLACK)
    lcd.draw_string(30, 150, "skewing:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 150, str(b), lcd.RED, lcd.BLACK)
    lcd.draw_string(50, 200, "flag:", lcd.GREEN, lcd.BLACK)
    lcd.draw_string(100, 200, str(flag), lcd.RED, lcd.BLACK)
    #print(a)
    print(a,b)
    if((a>=-30 and a<=30)and (b>=-30 and b<=38)):
        if a>25:
            a=25
        if a<-25:
            a=-25
        a+=100
        sending_data(a,flag)
    elif a>=30 and b>10:
        if b>40:
            b=40
        sending_data(-b*0.35+100,flag)
    elif a>=0 and b>38:
        sending_data(b*0.2+100,flag)
    elif b>0 and a>0:
        x=0.5*b
        if x>25:
            x=25
        if x<-25:
            x=-25
        sending_data(x+100,flag)
    else:
        y=0.5*b
        if y<-25:
            y=-25
        if y>25:
            y=25
        sending_data(y+100,flag)
