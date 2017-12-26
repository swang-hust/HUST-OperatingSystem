# HUST-OperatingSystem
2017 fall, codes about Operating System.

## Experiment 1
编写程序，演示多进程并发执行和进程软中断、管道通信。<br>
* 父进程使用系统调用pipe( )建立一个管道,然后使用系统调用fork()创建两个子进程，子进程1和子进程2；
* 子进程1每隔1秒通过管道向子进程2发送数据:<br>
	I send you x times. (x初值为1，每次发送后做加一操作）<br>
	子进程2从管道读出信息，并显示在屏幕上。<br>
* 父进程用系统调用signal()捕捉来自键盘的中断信号（即按Ctrl+C键）；当捕捉到中断信号后，父进程用系统调用Kill()向两个子进程发出信号，子进程捕捉到信号后分别输出下列信息后终止：<br>
  Child Process l is Killed by Parent!<br>
  Child Process 2 is Killed by Parent!<br>
* 父进程等待两个子进程终止后，释放管道并输出如下的信息后终止:<br>
  Parent Process is Killed!
