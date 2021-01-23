# QTL
QT源码剖析

## 功能列表

* [QSharedMemory](#QSharedMemory)
* QMap
* QHash
* QList

<a id="QSharedMemory"></a>

## QSharedMemory共享内存的实现

<p> It is a simple dialog that presents a few buttons. To compile the
  example, then run the executable twice
  to create two processes running the dialog. In one of the processes,
  press the button to load an image into a shared memory segment, and
  then select an image file to load. Once the first process has loaded
  and displayed the image, in the second process, press the button to
  read the same image from shared memory. The second process displays
  the same image loaded from its new loaction in shared memory.</p>