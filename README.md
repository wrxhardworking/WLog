# WLog日志库实现

该日志库是我借鉴sylar和muduo网络库所研究出来的具有高扩展性和高性能的跨平台的日志库。该日志库使用流式输出，只支持日志输出到文件，原因是网络io很不稳定，基本没有见过利用网络传输进行日志的打印。输出日志的文件名是根据程序运行的路径来自动取名的。release模式下平均每一条1.3us~1.4us之间，基本不会影响主程序的运行的性能。

[github仓库地址！！](https://github.com/wrxhardworking/c-Logging-library)
## 项目使用

 1. 下载源码将其添加为子目录
 2. 在cmake项目中添加以下命令：

```cpp
include(FetchContent)

FetchContent_Declare(
        WLog
        GIT_REPOSITORY https://github.com/wrxhardworking/WLog.git
        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(WLog)

target_link_libraries(${PROJECT_NAME} PRIVATE WLog)
```
这种方式是在在项目中将编译源码，链接库。
## 项目构建

```cmd
mkdir build && cd build
cmake ..
```

## 项目测试

```cpp
#include "../Logger.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

class Timer {
public:
    Timer() {
        m_curTime = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto startTime = std::chrono::time_point_cast<std::chrono::microseconds>(m_curTime)
                .time_since_epoch().count();
        auto endTime = std::chrono::time_point_cast<std::chrono::microseconds>(end)
                .time_since_epoch().count();
        auto duration = endTime - startTime;
        double ms = duration * 0.001;//得到毫秒
        printf("%lld us (%lf ms)\n", duration, ms);
        fflush(stdout);
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_curTime;
};

int main() {
    /*
     * part I:
     * 测试性能
     * part II:
     * 自定义日志名称，日志格式
     * part III:
     * 使用方式
     * part IV:
     * 线程安全的测试
    */
#if 1
    {
        Timer timer;
        {
            for (int i = 0; i < 10000; ++i) {
                LOG_INFO<<"hello world";
            }
        }
    }

#endif
#if 0
    //设置日志等级
    GET_LOGGER->setLevel(LogLever::ALERT);
    //设置日志名称
    GET_LOGGER->setName("service");
//    设置日志格式
    GET_LOGGER->setFormatter(std::make_shared<LogFormatter>("....."));
    //......敬请期待(指定日志输出地 默认名字是target_name 文件的默认路径是../build(构建目录))
#endif
#if 0
    /*
    * 1.支持普通字符串类型
    * 2.支持std::string类型
    * 3.支持std::vector<int>,std::vector<std::string>等类型
    */
    std::string log = "hello world ";
    std::vector<const char *> s{"hello"};
    LOG_INFO<<log<<s;
    LOG_INFO<<"HELLO WORLD";
    LOG_INFO<<std::vector<int>{1,2,3,4,5};
    LOG_INFO<<1.1111111;
    LOG_INFO<<111111111;
#endif
#if 0
    std::vector<std::thread> v;
    v.reserve(8);
for(int i = 0;i<100;i++){
        v.emplace_back([](){
            LOG_INFO<<"hello world";
        });
    }
for(auto &x :v){
    x.join();
}
#endif
}
```



这里我的电脑在release模式下测出打印100万条日志所需的的时间为1300ms，平均每一条1.3us~1.4us之间不同机子下应该有不同的性能。

## 项目总览
![在这里插入图片描述](https://img-blog.csdnimg.cn/3d17dae2486242f39df498218a607103.png#pic_center)



## 重点部分详解

### 1.init()

​		该方法是为了解析用户定义的格式，该格式包含模板字符和常规字符。模板字符：是指那些已经确定好了的意义的字符，它们分别代表一个属性，若是模板字符，则前面有我们规定好的转义字符%，例如：%c %d{%Y-%m-%d %H:%M:%S} %p 代表的则是 日志名 日期时间 日志等级 ，这里我们对d字符做了特别的处理，它代表日期时间，它的后面必须带有 {日期时间的格式化样式} 。常规字符：常规字符是真正会输出到日志文件中的实际内容。例如：%c %d{%Y-%m-%d %H:%M:%S} %p 线程id:%t %f %l %m %n  线程二字就会被实际输出到文件中。当然，还也可以在本库的基础上继续拓展，而且可以很规范的扩展，具体看源代码。

​		该方法是为了解析用户规定的输出日志的格式，采取的状态机的方法提取模板字符，将每个模板字符字符与FormatItem进行一个映射，进而得到具有正确顺序的m_items，重点在于利用状态机提取模板字符的算法。

代码如下（详细看源码）：

```cpp
void LogFormatter::init(){
    
    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<int,std::string>> patterns;

    // 临时存储常规字符串
    std::string tmp;

    //存储时间日期的格式
    std::string dateFormat;

    //是否解析出错
    bool error = false;

    //设置初始状态 默认为解析模板
    LogFormatter::Parse status = LogFormatter::Parse::NORMAL;

    size_t i = 0;
    while(i<m_pattern_.size()){
        //逐个字符进行解析
        std::string c(1,m_pattern_[i]);
        if(c=="%"){
            switch (status) {
            case LogFormatter::Parse::NORMAL:{
                if(!tmp.empty()){
                    //0表示常规字符
                    patterns.emplace_back(0,tmp);
                }
                tmp.clear();
                //因为解析常规字符的时候碰到了% 所以要解析模板字符了
                status = LogFormatter::Parse::TEMPLATE;
                break;
                }
            case LogFormatter::Parse::TEMPLATE:{
                //1表示模板字符
                //在解析模板字符的时候碰到了%说明%事项表示常规字符说明要解析常规字符了 要将其转为常规字符的状态
                patterns.emplace_back(1,c);
                status = LogFormatter::Parse::NORMAL;
                break;
                }
            }
            ++i;
            continue;
        }
        //碰到的不是%
        else{
            switch (status) {
            case LogFormatter::Parse::NORMAL:{
                //持续解析常规字符串 直到遇见% 把解析后的常规的字符串放入patterns中
                tmp += c;
                ++i;
                break;
                }
            case LogFormatter::Parse::TEMPLATE:{
                //若是解析模板字符
                patterns.emplace_back(1,c);
                //插入完后 将其切回常规字符
                status = LogFormatter::Parse::NORMAL;
                ++i;
                //此处又要对时间有特殊的处理 将%d后大括号中的内容放入dataformat
                if(c=="d"){
                    if(i<m_pattern_.size()&&m_pattern_[i]=='{'){
                        ++i;
                        while(i<m_pattern_.size()&&m_pattern_[i]!='}'){
                            dateFormat.push_back(m_pattern_[i]);
                            ++i;
                        }
                        //说明其中的大括号没有闭合
                        if(m_pattern_[i]!='}'){
                            std::cerr<<"parse dateFormat error"<<std::endl;
                            error = true;
                        }
                        //没有出错的此时i的位置是'}'的位置 所以要加一
                        ++i;
                    }
                }
                break;
                }
            }
            if(error) break;
            continue;
        }
    }
    
    static std::map<std::string,std::function<FormatterItem::ptr(const std::string &str)> > s_formatters = {
        //注意下面没有datetime的宏定义 它会在后面做一个特殊处理
        //这个宏处理直呼较好
    #define xx(str,func) \
    {#str,[](const std::string &fmt){return FormatterItem::ptr(new func(fmt));}}

        xx(m, MessageFormatItem),
        xx(p, LeverFormatItem),
        xx(c, NameFormatItem),
        xx(r, ElapseFormatItem),
        xx(f, FilenameFormatItem),
        xx(t, ThreadIdFormatItem),
        xx(F, FiberIdFormatItem),
        xx(n, NewLineFormatItem), 
        xx(l, LineFormatItem),
    #undef xx
    };
    for(auto&v : patterns){
        if(v.first==0){
            //是常规字符
            m_items_.emplace_back(FormatterItem::ptr(new StringFormaItem(v.second)));
        }
        // 是模板(转义字符)字符
        // 日期做特殊的处理
        else if(v.second == "d"){
            m_items_.emplace_back(FormatterItem::ptr(new DateTimeFormatItem(dateFormat)));
        }
        //以下是模板字符的处理
        else{
            auto it = s_formatters.find(v.second);
                if(it == s_formatters.end()){
                    std::cerr<<"error happend is "<<v.second<< std::endl;
                    error = true;
                    break;
                }
                else{
                    //map中存在该字符对应的Item
                    m_items_.emplace_back(it->second(v.second));
                }
        }
    }
}
```

### 2.双缓冲技术

​		基本思路是准备两个buffer：A与B，前端负责将A填数据，后端则负责B中的数据写入文件。当A满了则交换A，B，如此往复。这样做的好处：1不用直接向磁盘写消息；2也避免每条日志都触发后端日志线程，减少了线程唤醒的频率降低开销。为了及时将日志文件写入消息，即便A没有满，日志库也每隔3s执行一次交换操作。在实际实现中我们采取了四个缓冲区，这样可以进一步避免日志前端的等待。每个容器（缓冲区）大小为4MB，至少可以放1000条日志消息，std::unique_ptr具备移动语义，而且可以自己管理生命周期。

​		前端来一条日志如果当前缓冲区大小足够则直接append，这里拷贝一条日志消息不会造成多大开销，因为其他部分都是对指针的操作。如果当前缓冲区大小都不够了，则将当前缓冲区移入数组，并将另外一个块准备好的缓冲区作为当前缓冲区，然后追加日志消息后端开始写日志。如果前端写日志很快一下子把两块缓冲区都使用完了，那么只好重新分配一块缓冲区，这种情况很少见。双缓冲技术精妙之处是临界区的缩小。具体看源代码

代码如下图：

```cpp
//后台线程所作的事情
void AsyncLogging::backendThread() {
    //以追加的方式打开一个文件 base为文件路径
    FILE* fp = fopen(base_,"a+");
    //以下两块buffer是为后面的更替做准备
    BufferPtr replaceBuffer1(new Buffer);
    BufferPtr replaceBuffer2(new Buffer);
    //初始化
    replaceBuffer1->beZero();
    replaceBuffer2->beZero();
    //输出到文件中buffer 用一个容器装起
    Buffers BufferToWrite;
    //预留空间 省去打日志时申请时间
    BufferToWrite.reserve(16);
    while(running_){
        //以下是临界区处
        {
            std::unique_lock<std::mutex> locker(mutex_);
            //fixme 会不会存在虚假唤醒? buffers不为空或者等待事件超过三秒
            if (buffers_.empty()) {
                condition_.wait_for(locker, std::chrono::seconds(flushTimeInterval_));
            }
            //超时阶段
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(replaceBuffer1);
            BufferToWrite.swap(buffers_);
            //如果预备buffer被占用
            if (!nextBuffer_) {
                nextBuffer_ = std::move(replaceBuffer2);
            }
        }

        for(const auto & buffer : BufferToWrite){
            fwrite(buffer->data(),1,buffer->length(),fp);
        }
        //一般只有一个
        if(BufferToWrite.size()>2){
            BufferToWrite.resize(2);
        }
        if(!replaceBuffer1){
            replaceBuffer1 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
        }
        if(!replaceBuffer2){
            replaceBuffer2 = std::move(BufferToWrite.back());
            BufferToWrite.pop_back();
        }

        //将写缓冲区置空
        BufferToWrite.clear();
        // fixme flush
        fflush(fp);
    }
    //fixme flush
    fflush(fp);

    if(fp){
        fclose(fp);
    }
}
```


