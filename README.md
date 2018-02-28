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
</div><div id="MySignature"></div>
<div class="clear"></div>
<div id="blog_post_info_block">
<div id="BlogPostCategory"></div>
<div id="EntryTag"></div>
<div id="blog_post_info"><div id="green_channel">
        <a href="javascript:void(0);" id="green_channel_digg" onclick="DiggIt(6376508,cb_blogId,1);green_channel_success(this,'谢谢推荐！');">好文要顶</a>
            <a id="green_channel_follow" onclick="follow('de96944d-72de-e611-845c-ac853d9f53ac');" href="javascript:void(0);">关注我</a>
    <a id="green_channel_favorite" onclick="AddToWz(cb_entryId);return false;" href="javascript:void(0);">收藏该文</a>
    <a id="green_channel_weibo" href="javascript:void(0);" title="分享至新浪微博" onclick="ShareToTsina()"><img src="//common.cnblogs.com/images/icon_weibo_24.png" alt=""></a>
    <a id="green_channel_wechat" href="javascript:void(0);" title="分享至微信" onclick="shareOnWechat()"><img src="//common.cnblogs.com/images/wechat.png" alt=""></a>
</div>
<div id="author_profile">
    <div id="author_profile_info" class="author_profile_info">
            <a href="http://home.cnblogs.com/u/euphie/" target="_blank"><img src="//pic.cnblogs.com/face/sample_face.gif" class="author_avatar" alt=""></a>
        <div id="author_profile_detail" class="author_profile_info">
            <a href="http://home.cnblogs.com/u/euphie/">Euphie</a><br>
            <a href="http://home.cnblogs.com/u/euphie/followees">关注 - 0</a><br>
            <a href="http://home.cnblogs.com/u/euphie/followers">粉丝 - 1</a>
        </div>
    </div>
    <div class="clear"></div>
    <div id="author_profile_honor"></div>
    <div id="author_profile_follow">
                <a href="javascript:void(0);" onclick="follow('de96944d-72de-e611-845c-ac853d9f53ac');return false;">+加关注</a>
    </div>
</div>
<div id="div_digg">
    <div class="diggit" onclick="votePost(6376508,'Digg')">
        <span class="diggnum" id="digg_count">1</span>
    </div>
    <div class="buryit" onclick="votePost(6376508,'Bury')">
        <span class="burynum" id="bury_count">0</span>
    </div>
    <div class="clear"></div>
    <div class="diggword" id="digg_tips">
    </div>
</div>
<script type="text/javascript">
    currentDiggType = 0;
</script></div>
<div class="clear"></div>
<div id="post_next_prev"><a href="http://www.cnblogs.com/euphie/p/7008077.html" class="p_n_p_prefix">» </a> 下一篇：<a href="http://www.cnblogs.com/euphie/p/7008077.html" title="发布于2017-06-14 11:53">进程、线程和协程</a><br></div>
</div>


		</div>
		<div class="postDesc">posted @ <span id="post-date">2017-02-08 02:47</span> <a href="http://www.cnblogs.com/euphie/">Euphie</a> 阅读(<span id="post_view_count">1428</span>) 评论(<span id="post_comment_count">4</span>)  <a href="https://i.cnblogs.com/EditPosts.aspx?postid=6376508" rel="nofollow">编辑</a> <a href="#" onclick="AddToWz(6376508);return false;">收藏</a></div>
	</div>
