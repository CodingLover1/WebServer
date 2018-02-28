# WebServer
>This is a Web Server.It just for learing Network Programming.

### How to use it
* make a dir in /var.like this /var/www/html
* then make a file like index.html under the /var/www/html
* compile the server:make 
* in the browser input http://ip:port/index.html

### unix线程模型



[转载](https://www.cnblogs.com/euphie/p/6376508.html)

<div class="post">
		<h1 class="postTitle">
			<a id="cb_post_title_url" class="postTitle2" href="http://www.cnblogs.com/euphie/p/6376508.html">同步IO、异步IO、阻塞IO、非阻塞IO之间的联系与区别</a>
		</h1>
		<div class="clear"></div>
		<div class="postBody">
			<div id="cnblogs_post_body" class="blogpost-body cnblogs-markdown"><h2 id="posix">POSIX</h2>
<p>同步IO、异步IO、阻塞IO、非阻塞IO，这几个词常见于各种各样的与网络相关的文章之中，往往不同上下文中它们的意思是不一样的，以致于我在很长一段时间对此感到困惑，所以想写一篇文章整理一下。</p>
<pre><code class="hljs">POSIX(可移植操作系统接口)把同步IO操作定义为导致进程阻塞直到IO完成的操作，反之则是异步IO</code></pre>
<p>按POSIX的描述似乎把同步和阻塞划等号，异步和非阻塞划等号，但是为什么有的人说同步IO不等于阻塞IO呢？先来说说几种常见的IO模型吧。</p>
<h2 id="io模型">IO模型</h2>
<p>这里统一使用Linux下的系统调用recv作为例子，它用于从套接字上接收一个消息，因为是一个系统调用，所以调用时会<strong>从用户进程空间切换到内核空间运行一段时间再切换回来</strong>。默认情况下recv会等到网络数据到达并且复制到用户进程空间或者发生错误时返回，而第4个参数flags可以让它马上返回。</p>
<ul>
<li><strong>阻塞IO模型</strong></li>
</ul>
<p>使用recv的默认参数一直等数据直到拷贝到用户空间，这段时间内进程始终阻塞。A同学用杯子装水，打开水龙头装满水然后离开。这一过程就可以看成是使用了阻塞IO模型，因为如果水龙头没有水，他也要等到有水并装满杯子才能离开去做别的事情。很显然，这种IO模型是同步的。</p>
<p><img src="http://image.euphie.net/2017-09-24-23-18-01.png" alt="image"></p>
<ul>
<li><strong>非阻塞IO模型</strong></li>
</ul>
<p>改变flags，让recv不管有没有获取到数据都返回，如果没有数据那么一段时间后再调用recv看看，如此循环。B同学也用杯子装水，打开水龙头后发现没有水，它离开了，过一会他又拿着杯子来看看……在中间离开的这些时间里，B同学离开了装水现场(回到用户进程空间)，可以做他自己的事情。这就是非阻塞IO模型。但是它只有是检查无数据的时候是非阻塞的，在数据到达的时候依然要等待复制数据到用户空间(等着水将水杯装满)，因此它还是同步IO。</p>
<p><img src="http://image.euphie.net/2017-09-24-23-19-53.png" alt="image"></p>
<ul>
<li><strong>IO复用模型</strong></li>
</ul>
<p>这里在调用recv前先调用select或者poll，这2个系统调用都可以在内核准备好数据(网络数据到达内核)时告知用户进程，这个时候再调用recv一定是有数据的。因此这一过程中它是阻塞于select或poll，而没有阻塞于recv，有人将非阻塞IO定义成在读写操作时没有阻塞于系统调用的IO操作(不包括数据从内核复制到用户空间时的阻塞，因为这相对于网络IO来说确实很短暂)，如果按这样理解，这种IO模型也能称之为非阻塞IO模型，但是按POSIX来看，它也是同步IO，那么也和楼上一样称之为同步非阻塞IO吧。</p>
<p>这种IO模型比较特别，分个段。因为它能同时监听多个文件描述符(fd)。这个时候C同学来装水，发现有一排水龙头，舍管阿姨告诉他这些水龙头都还没有水，等有水了告诉他。于是等啊等(select调用中)，过了一会阿姨告诉他有水了，但不知道是哪个水龙头有水，自己看吧。于是C同学一个个打开，往杯子里装水(recv)。这里再顺便说说鼎鼎大名的epoll(高性能的代名词啊)，epoll也属于IO复用模型，主要区别在于舍管阿姨会告诉C同学哪几个水龙头有水了，不需要一个个打开看(当然还有其它区别)。</p>
<p><img src="http://image.euphie.net/2017-09-24-23-21-54.png" alt="image"></p>
<ul>
<li><strong>信号驱动IO模型</strong></li>
</ul>
<p>通过调用sigaction注册信号函数，等内核数据准备好的时候系统中断当前程序，执行信号函数(在这里面调用recv)。D同学让舍管阿姨等有水的时候通知他(注册信号函数)，没多久D同学得知有水了，跑去装水。是不是很像异步IO？很遗憾，它还是同步IO(省不了装水的时间啊)。</p>
<p><img src="http://image.euphie.net/2017-09-24-23-22-38.png" alt="image"></p>
<ul>
<li><strong>异步IO模型</strong></li>
</ul>
<p>调用aio_read，让内核等数据准备好，并且复制到用户进程空间后执行事先指定好的函数。E同学让舍管阿姨将杯子装满水后通知他。整个过程E同学都可以做别的事情(没有recv)，这才是真正的异步IO。</p>
<p><img src="http://image.euphie.net/2017-09-24-23-23-36.png" alt="image"></p>
<h2 id="总结">总结</h2>
<p>IO分两阶段：</p>
<pre><code class="hljs">1.数据准备阶段
2.内核空间复制回用户进程缓冲区阶段</code></pre>
<p>一般来讲：阻塞IO模型、非阻塞IO模型、IO复用模型(select/poll/epoll)、信号驱动IO模型都属于同步IO，因为阶段2是阻塞的(尽管时间很短)。只有异步IO模型是符合POSIX异步IO操作含义的，不管在阶段1还是阶段2都可以干别的事。</p>
<ul>
<li>ps：以上图片均截自UNIX网络编程卷1。</li>
</ul>
</div>

<div id="article_content" class="article_content csdn-tracking-statistics tracking-click" data-mod="popu_519" data-dsm="post" style="overflow: hidden;">
                            <div class="markdown_views">
                        <h2 id="关于io多路复用"><a name="t0"></a>关于I/O多路复用：</h2>

<p>I/O多路复用(又被称为“事件驱动”)，首先要理解的是，操作系统为你提供了一个功能，当你的某个socket可读或者可写的时候，它可以给你一个通知。这样当配合非阻塞的socket使用时，只有当系统通知我哪个描述符可读了，我才去执行read操作，可以保证每次read都能读到有效数据而不做纯返回-1和EAGAIN的无用功。写操作类似。操作系统的这个功能通过select/poll/epoll之类的系统调用来实现，这些函数都可以同时监视多个描述符的读写就绪状况，这样，**多个描述符的I/O操作都能在一个线程内并发交替地顺序完成，这就叫I/O多路复用，这里的“复用”指的是复用同一个线程。</p>



<h2 id="一io复用之select"><a name="t1"></a>一、I/O复用之select</h2>

<p>1、介绍： <br>
select系统调用的目的是：在一段指定时间内，监听用户感兴趣的文件描述符上的可读、可写和异常事件。poll和select应该被归类为这样的系统调用，它们可以阻塞地同时探测一组支持非阻塞的IO设备，直至某一个设备触发了事件或者超过了指定的等待时间——也就是说它们的职责不是做IO，而是帮助调用者寻找当前就绪的设备。 <br>
    下面是select的原理图： <br>
    <img src="http://img.blog.csdn.net/20160422141110022" alt="这里写图片描述" title=""></p>

<p>2、select系统调用API如下：</p>



<pre class="prettyprint"><code class="hljs perl has-numbering"><span class="hljs-comment">#include &lt;sys/time.h&gt;</span>
<span class="hljs-comment">#include &lt;sys/types.h&gt;</span>
<span class="hljs-comment">#include &lt;unistd.h&gt;</span>
<span class="hljs-keyword">int</span> <span class="hljs-keyword">select</span>(<span class="hljs-keyword">int</span> nfds, fd_set <span class="hljs-variable">*readfds</span>, fd_set <span class="hljs-variable">*writefds</span>, fd_set <span class="hljs-variable">*exceptfds</span>, struct timeval <span class="hljs-variable">*timeout</span>);</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li><li style="color: rgb(153, 153, 153);">4</li></ul></pre>

<p>fd_set结构体是文件描述符集，该结构体实际上是一个整型数组，数组中的每个元素的每一位标记一个文件描述符。fd_set能容纳的文件描述符数量由FD_SETSIZE指定，一般情况下，FD_SETSIZE等于1024，这就限制了select能同时处理的文件描述符的总量。</p>

<p>3、下面介绍一下各个参数的含义： <br>
    1）nfds参数指定被监听的文件描述符的总数。通常被设置为select监听的所有文件描述符中最大值加1； <br>
    2）readfds、writefds、exceptfds分别指向可读、可写和异常等事件对应的文件描述符集合。这三个参数都是传入传出型参数，指的是在调用select之前，用户把关心的可读、可写、或异常的文件描述符通过FD_SET（下面介绍）函数分别添加进readfds、writefds、exceptfds文件描述符集，select将对这些文件描述符集中的文件描述符进行监听，如果有就绪文件描述符，<strong>select会重置readfds、writefds、exceptfds文件描述符集来通知应用程序哪些文件描述符就绪。这个特性将导致select函数返回后，再次调用select之前，必须重置我们关心的文件描述符</strong>，也就是三个文件描述符集已经不是我们之前传入 的了。 <br>
    3）timeout参数用来指定select函数的超时时间（下面讲select返回值时还会谈及）。</p>



<pre class="prettyprint"><code class="hljs cs has-numbering"><span class="hljs-keyword">struct</span> timeval
{
    <span class="hljs-keyword">long</span> tv_sec;        <span class="hljs-comment">//秒数</span>
    <span class="hljs-keyword">long</span> tv_usec;       <span class="hljs-comment">//微秒数</span>
};</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li><li style="color: rgb(153, 153, 153);">4</li><li style="color: rgb(153, 153, 153);">5</li></ul></pre>

<p>4、下面几个函数（宏实现）用来操纵文件描述符集：</p>



<pre class="prettyprint"><code class="hljs cs has-numbering"><span class="hljs-keyword">void</span> FD_SET(<span class="hljs-keyword">int</span> fd, fd_set *<span class="hljs-keyword">set</span>);   <span class="hljs-comment">//在set中设置文件描述符fd</span>
<span class="hljs-keyword">void</span> FD_CLR(<span class="hljs-keyword">int</span> fd, fd_set *<span class="hljs-keyword">set</span>);   <span class="hljs-comment">//清除set中的fd位</span>
<span class="hljs-keyword">int</span>  FD_ISSET(<span class="hljs-keyword">int</span> fd, fd_set *<span class="hljs-keyword">set</span>); <span class="hljs-comment">//判断set中是否设置了文件描述符fd</span>
<span class="hljs-keyword">void</span> FD_ZERO(fd_set *<span class="hljs-keyword">set</span>);          <span class="hljs-comment">//清空set中的所有位（在使用文件描述符集前，应该先清空一下）</span>
    <span class="hljs-comment">//（注意FD_CLR和FD_ZERO的区别，一个是清除某一位，一个是清除所有位）</span></code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li><li style="color: rgb(153, 153, 153);">4</li><li style="color: rgb(153, 153, 153);">5</li></ul></pre>

<p>5、select的返回情况： <br>
    1）如果指定timeout为NULL，select会永远等待下去，直到有一个文件描述符就绪，select返回； <br>
    2）如果timeout的指定时间为0，select根本不等待，立即返回； <br>
    3）如果指定一段固定时间，则在这一段时间内，如果有指定的文件描述符就绪，select函数返回，如果超过指定时间，select同样返回。 <br>
    4）返回值情况： <br>
        a)超时时间内，如果文件描述符就绪，select返回就绪的文件描述符总数（包括可读、可写和异常），如果没有文件描述符就绪，select返回0； <br>
        b)select调用失败时，返回 -1并设置errno，如果收到信号，select返回 -1并设置errno为EINTR。</p>

<p>6、文件描述符的就绪条件： <br>
在网络编程中， <br>
    1）<strong>下列情况下socket可读：</strong> <br>
        a) socket内核接收缓冲区的字节数大于或等于其低水位标记SO_RCVLOWAT； <br>
        b) socket通信的对方关闭连接，此时该socket可读，但是一旦读该socket，会立即返回0（可以用这个方法判断client端是否断开连接）； <br>
        c) 监听socket上有新的连接请求； <br>
        d) socket上有未处理的错误。 <br>
    2）<strong>下列情况下socket可写：</strong> <br>
        a) socket内核发送缓冲区的可用字节数大于或等于其低水位标记SO_SNDLOWAT； <br>
        b) socket的读端关闭，此时该socket可写，一旦对该socket进行操作，该进程会收到SIGPIPE信号； <br>
        c) socket使用connect连接成功之后； <br>
        d) socket上有未处理的错误。</p>

<h2 id="二io复用之poll"><a name="t2"></a>二、I/O复用之poll</h2>

<p>1、poll系统调用的原理与原型和select基本类似，也是在指定时间内<strong>轮询</strong>一定数量的文件描述符，以测试其中是否有就绪者。</p>

<p>2、poll系统调用API如下：</p>



<pre class="prettyprint"><code class="hljs vala has-numbering"><span class="hljs-preprocessor">#include &lt;poll.h&gt;</span>
<span class="hljs-keyword">int</span> poll(<span class="hljs-keyword">struct</span> pollfd *fds, nfds_t nfds, <span class="hljs-keyword">int</span> timeout);</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li></ul></pre>

<p>3、下面介绍一下各个参数的含义： <br>
    1）第一个参数是指向一个结构数组的第一个元素的指针，每个元素都是一个pollfd结构，用于指定测试某个给定描述符的条件。</p>



<pre class="prettyprint"><code class="hljs cs has-numbering"><span class="hljs-keyword">struct</span> pollfd
{
    <span class="hljs-keyword">int</span> fd;             <span class="hljs-comment">//指定要监听的文件描述符</span>
    <span class="hljs-keyword">short</span> events;       <span class="hljs-comment">//指定监听fd上的什么事件</span>
    <span class="hljs-keyword">short</span> revents;      <span class="hljs-comment">//fd上事件就绪后，用于保存实际发生的时间</span>
}；</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li><li style="color: rgb(153, 153, 153);">4</li><li style="color: rgb(153, 153, 153);">5</li><li style="color: rgb(153, 153, 153);">6</li></ul></pre>

<p>待监听的事件由events成员指定，函数在相应的revents成员中返回该描述符的状态（每个文件描述符都有两个事件，一个是传入型的events，一个是传出型的revents，从而避免使用传入传出型参数，注意与select的区别），从而告知应用程序fd上实际发生了哪些事件。<strong>events和revents都可以是多个事件的按位或。</strong> <br>
    2）第二个参数是要监听的文件描述符的个数，也就是数组fds的元素个数； <br>
    3）第三个参数意义与select相同。</p>

<p>4、poll的事件类型： <br>
<img src="http://img.blog.csdn.net/20160422142729580" alt="这里写图片描述" title=""> <br>
在使用POLLRDHUP时，要在代码开始处定义_GNU_SOURCE</p>

<p>5、poll的返回情况： <br>
        与select相同。</p>



<h2 id="三io复用之epoll"><a name="t3"></a>三、I/O复用之epoll</h2>

<p>1、介绍： <br>
    epoll 与select和poll在使用和实现上有很大区别。首先，epoll使用一组函数来完成，而不是单独的一个函数；其次，epoll把用户关心的文件描述符上的事件放在内核里的一个事件表中，无须向select和poll那样每次调用都要重复传入文件描述符集合事件集。</p>

<p>2、创建一个文件描述符，指定内核中的事件表：</p>



<pre class="prettyprint"><code class="hljs vala has-numbering"><span class="hljs-preprocessor">#include&lt;sys/epoll.h&gt;</span>
<span class="hljs-keyword">int</span> epoll_create(<span class="hljs-keyword">int</span> size);
    <span class="hljs-comment">//调用成功返回一个文件描述符，失败返回-1并设置errno。</span></code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li></ul></pre>

<p>size参数并不起作用，只是给内核一个提示，告诉它事件表需要多大。<strong>该函数返回的文件描述符指定要访问的内核事件表，是其他所有epoll系统调用的句柄。</strong></p>

<p>3、操作内核事件表：</p>



<pre class="prettyprint"><code class="hljs cs has-numbering"><span class="hljs-preprocessor">#include&lt;sys/epoll.h&gt;</span>
<span class="hljs-keyword">int</span> epoll_ctl(<span class="hljs-keyword">int</span> epfd, <span class="hljs-keyword">int</span> op, <span class="hljs-keyword">int</span> fd, <span class="hljs-keyword">struct</span> epoll_event *<span class="hljs-keyword">event</span>);
    <span class="hljs-comment">//调用成功返回0，调用失败返回-1并设置errno。</span></code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li></ul></pre>

<p>epfd是epoll_create返回的文件句柄，标识事件表，op指定操作类型。操作类型有以下3种：</p>



<pre class="prettyprint"><code class="hljs livecodeserver has-numbering"><span class="hljs-operator">a</span>）EPOLL_CTL_ADD， 往事件表中注册fd上的事件；
b）EPOLL_CTL_MOD， 修改fd上注册的事件；
c）EPOLL_CTL_DEL， 删除fd上注册的事件。</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li></ul></pre>

<p>event参数指定事件，epoll_event的定义如下：</p>



<pre class="prettyprint"><code class="hljs d has-numbering"><span class="hljs-keyword">struct</span> epoll_event
{
    __int32_t events;       <span class="hljs-comment">//epoll事件</span>
    epoll_data_t data;      <span class="hljs-comment">//用户数据</span>
};

<span class="hljs-keyword">typedef</span> <span class="hljs-keyword">union</span> epoll_data
{
    <span class="hljs-keyword">void</span> *ptr;
    <span class="hljs-keyword">int</span>  fd;
    uint32_t u32;
    uint64_t u64;
}epoll_data;</code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li><li style="color: rgb(153, 153, 153);">4</li><li style="color: rgb(153, 153, 153);">5</li><li style="color: rgb(153, 153, 153);">6</li><li style="color: rgb(153, 153, 153);">7</li><li style="color: rgb(153, 153, 153);">8</li><li style="color: rgb(153, 153, 153);">9</li><li style="color: rgb(153, 153, 153);">10</li><li style="color: rgb(153, 153, 153);">11</li><li style="color: rgb(153, 153, 153);">12</li><li style="color: rgb(153, 153, 153);">13</li></ul></pre>

<p>在使用epoll_ctl时，是把fd添加、修改到内核事件表中，或从内核事件表中删除fd的事件。如果是添加事件到事件表中，可以往data中的fd上添加事件events，或者不用data中的fd，而把fd放到用户数据ptr所指的内存中（因为epoll_data是一个联合体，只能使用其中一个数据）,再设置events。</p>

<p>3、epoll_wait函数 <br>
epoll系统调用的最关键的一个函数epoll_wait，它在一段时间内等待一个组文件描述符上的事件。</p>



<pre class="prettyprint"><code class="hljs vala has-numbering"><span class="hljs-preprocessor">#include&lt;sys/epoll.h&gt;</span>
<span class="hljs-keyword">int</span> epoll_wait(<span class="hljs-keyword">int</span> epfd, <span class="hljs-keyword">struct</span> epoll_event *events, <span class="hljs-keyword">int</span> maxevents, <span class="hljs-keyword">int</span> timeout);
    <span class="hljs-comment">//函数调用成功返回就绪文件描述符个数，失败返回-1并设置errno。</span></code><ul class="pre-numbering" style=""><li style="color: rgb(153, 153, 153);">1</li><li style="color: rgb(153, 153, 153);">2</li><li style="color: rgb(153, 153, 153);">3</li></ul></pre>

<p>timeout参数和select与poll相同，指定一个超时时间；maxevents指定最多监听多少个事件；events是一个传出型参数，epoll_wait函数如果检测到事件就绪，就将所有就绪的事件从内核事件表（epfd所指的文件）中复制到events指定的数组中。这个数组用来输出epoll_wait检测到的就绪事件，而不像select与poll那样，这也是epoll与前者最大的区别，下文在比较三者之间的区别时还会说到。 </p>



<h2 id="四三组io复用函数的比较"><a name="t4"></a>四、三组I/O复用函数的比较</h2>

<p><strong>相同点：</strong> <br>
1）三者都需要在fd上注册用户关心的事件； <br>
2）三者都要一个timeout参数指定超时时间； <br>
<strong>不同点：</strong> <br>
1）select： <br>
    a）select指定三个文件描述符集，分别是可读、可写和异常事件，所以不能更加细致地区分所有可能发生的事件； <br>
    b）select如果检测到就绪事件，会在原来的文件描述符上改动，以告知应用程序，文件描述符上发生了什么时间，所以<strong>再次调用select时，必须先重置文件描述符</strong>； <br>
    c）select采用对所有注册的文件描述符集轮询的方式，会返回整个用户注册的事件集合，所以应用程序索引就绪文件的时间复杂度为O(n)； <br>
    d）select允许监听的最大文件描述符个数通常有限制，一般是1024，如果大于1024，select的性能会急剧下降； <br>
    e）只能工作在LT模式。</p>

<p>2）poll： <br>
    a）poll把文件描述符和事件绑定，事件不但可以单独指定，而且可以是多个事件的按位或，这样更加细化了事件的注册，而且poll单独采用一个元素用来保存就绪返回时的结果，这样在下次调用poll时，就不用重置之前注册的事件； <br>
    b）poll采用对所有注册的文件描述符集轮询的方式，会返回整个用户注册的事件集合，所以应用程序索引就绪文件的时间复杂度为O(n)。 <br>
    c）poll用nfds参数指定最多监听多少个文件描述符和事件，这个数能达到系统允许打开的最大文件描述符数目，即65535。 <br>
    d）只能工作在LT模式。</p>

<p>3）epoll： <br>
    a）epoll把用户注册的文件描述符和事件放到内核当中的事件表中，提供了一个独立的系统调用epoll_ctl来管理用户的事件，而且epoll采用回调的方式，一旦有注册的文件描述符就绪，讲触发回调函数，该回调函数将就绪的文件描述符和事件拷贝到用户空间events所管理的内存，这样应用程序索引就绪文件的时间复杂度达到O(1)。 <br>
    b）epoll_wait使用maxevents来制定最多监听多少个文件描述符和事件，这个数能达到系统允许打开的最大文件描述符数目，即65535； <br>
    c）不仅能工作在LT模式，而且还支持ET高效模式（即EPOLLONESHOT事件，读者可以自己查一下这个事件类型，对于epoll的线程安全有很好的帮助）。</p>

<p>select/poll/epoll总结： <br>
<img src="http://img.blog.csdn.net/20160422143642896" alt="这里写图片描述" title=""></p>                </div>
                                                <link rel="stylesheet" href="http://csdnimg.cn/release/phoenix/production/markdown_views-0bc64ada25.css">
                                    </div>


		
