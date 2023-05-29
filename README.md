
# WLog日志库

​		该日志库是我自己借鉴其他日志库的优点及自己的一些想法所构建的日志库用于平常项目中的使用。

[github仓库地址！！](https://github.com/wrxhardworking/c-Logging-library)

## 项目总览
![在这里插入图片描述](https://img-blog.csdnimg.cn/ca08ffd84ade4e08a9bca6920a5862e2.png#pic_center)


## 项目特点

1. 流式输出
2. 并发安全
3. 跨平台
4. 日志输出到文件和控制台（可自由选择）
5. 用户格式可自定义
6. 多种类型输入（可拓展）
7. 拓展性极高
8. 性能优（进行了性能优化，release下输出到文件为600ns~700ns一条）
9. 线程独立性日志（不同工作线程有独立的日志器）

## 默认配置

**默认日志名称**：root

**默认日志格式**：%c %d{%Y-%m-%d %H:%M:%S} %p %t %f %l %m %n ：

日志名称 日期（年月日时分秒）日志等级 线程id 所属目录文件路径 文件下的行号 实际日志信息 换行符  

**默认日志等级**：DEBUG

## 引入库

环境要求：cmake_minimum_required 3.0.0 ，compiler support c++17.

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

## 库使用

```cpp
#include "../include/Wlog.h"

#include <mutex>
#include <chrono>
#include <thread>

//定时器 用于测试性能
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
        printf("%ld us (%lf ms)\n", duration, ms);
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
    //除去第一条初始化的时间
    WLOG_LOG_DEBUG << "";
    WLOG_GET_LOGGER->setIsStdout(false);
    {
        for (int j = 0; j < 5; ++j) {
            Timer timer;
            {
                for (int i = 0; i < 1000000; ++i) {
                    WLOG_LOG_DEBUG << "hello world";
                }
            }
        }
    }

#endif
#if 0
    //设置日志等级
    WLOG_GET_LOGGER->setLevel(LogLever::ALERT);
    //设置日志名称
    WLOG_GET_LOGGER->setName("service");
    //设置日志格式
    WLOG_GET_LOGGER->setFormatter(".....");
    //设置输出地限制
    WLOG_GET_LOGGER->setIsFile(false);
    WLOG_GET_LOGGER->setIsStdout(false);
    //......敬请期待(指定日志输出地 默认名字是target_name 文件的默认路径是../build(构建目录))
#endif
#if 0
    /*
    * 1.支持普通字符串类型
    * 2.支持std::string类型
    * 3.支持std::vector<int>,std::vector<std::string>等类型
    * 4.支持打印地址(十进制进行打印)
    */
    int a = 100;
    std::string log = "hello world ";
    std::vector<const char *> s{"hello"};
    WLOG_LOG_INFO<<log<<s;
    WLOG_LOG_INFO<<"HELLO WORLD";
    WLOG_LOG_INFO<<std::vector<int>{1,2,3,4,5};
    WLOG_LOG_INFO<<1.1111111;
    WLOG_LOG_INFO<<111111111;
    WLOG_LOG_INFO<<&a;
#endif
#if 0
    std::vector<std::thread> v;
    v.reserve(8);
    for (int i = 0; i < 8; i++) {
        v.emplace_back([=]() {
            WLOG_GET_LOGGER->setName("test_logger_" + std::to_string(i));
            WLOG_LOG_INFO << "hello world";
        });
    }
    for (auto &x: v) {
        x.join();
    }
#endif
}
```

## 性能测试

本机配置：

处理器：Intel(R) Core(TM) i7-10510U CPU @ 1.80GHz   2.30 GHz

内存   ：16.0 GB

5次100万条日志输出文件测试：

691384 us (691.384000 ms)

696132 us (696.132000 ms)

688315 us (688.315000 ms)

696166 us (696.166000 ms)

675456 us (675.456000 ms)

## 项目特色技巧

  - 使用状态机模式解析日志格式，包含日志名称，日志等级，日志内容，线程id，协程id，日期，所属文件及所属行号。
  - 资源统一采用智能指针进行管理，避免了悬空指针，内存泄漏的问题。
  - 充分了利用了宏的特性，减少了代码量，区分操作系统，给用户提供友好的接口。
  - 文件输入中采用了双缓冲的技术，极大地缩短的临界区和刷盘次数，提高了性能。
  - 充分利用了面向对象的特性，对各个模块有良好的封装，使项目后期具有极高的拓展性。
  - 利用了template的SFINAE和函数重载的特性，使得日志内容类型多样化。
  - 利用RAII机制和thread_local变量，避免了很多多线程下并发安全的问题。比如：利用RAII机制，使用临时对象的构造和析构完成对一条日志的操作。

## 重点部分详解

### 1.状态机解析日志格式

​		该方法是为了解析用户定义的格式，该格式包含模板字符和常规字符。模板字符：是指那些已经确定好了的意义的字符，它们分别代表一个属性，若是模板字符，则前面有我们规定好的转义字符%，例如：%c %d{%Y-%m-%d %H:%M:%S} %p 代表的则是 日志名 日期时间 日志等级 ，这里我们对d字符做了特别的处理，它代表日期时间，它的后面必须带有 {日期时间的格式化样式} 。常规字符：常规字符是真正会输出到日志文件中的实际内容。例如：%c %d{%Y-%m-%d %H:%M:%S} %p 线程id:%t %f %l %m %n  线程二字就会被实际输出到文件中。当然，还也可以在本库的基础上继续拓展，而且可以很规范的扩展，具体看源代码。

​		该方法是为了解析用户规定的输出日志的格式，采取的状态机的方法提取模板字符，将每个模板字符字符与FormatItem进行一个映射，进而得到具有正确顺序的m_items，重点在于利用状态机提取模板字符的算法。其中有一个宏的技巧很妙：

代码如下（详细看源码）：

- ```cpp
   static std::map<std::string,std::function<FormatterItem::ptr(const std::string &str)> > s_formatters = {
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
  ```


### 2.双缓冲技术

​	基本思路是准备两个buffer：A与B，前端负责将A填数据，后端则负责B中的数据写入文件。当A满了则交换A，B，如此往复。这样做的好处：1.不用直接向磁盘写消息；2.也避免每条日志都触发后端日志线程，减少了线程唤醒的频率降低开销。为了及时将日志文件写入消息，即便A没有满，日志库也每隔3s执行一次交换操作。在实际实现中我们采取了四个缓冲区，这样可以进一步避免日志前端的等待。每个容器（缓冲区）大小为4MB，至少可以放1000条日志消息，std::unique_ptr具备移动语义，而且可以自己管理生命周期。

​	前端来一条日志如果当前缓冲区大小足够则直接append，这里拷贝一条日志消息不会造成多大开销，因为其他部分都是对指针的操作。如果当前缓冲区大小都不够了，则将当前缓冲区移入数组，并将另外一个块准备好的缓冲区作为当前缓冲区，然后追加日志消息后端开始写日志。如果前端写日志很快一下子把两块缓冲区都使用完了，那么只好重新分配一块缓冲区，这种情况很少见。双缓冲技术精妙之处是临界区的缩小。具体看源代码 : /src/AsyncLog.cpp

关键代码：

```cpp
        //以下是临界区处
        {
            std::unique_lock<std::mutex> locker(Mutex);
            //fixme 会不会存在虚假唤醒? buffers不为空或者等待事件超过三秒
            if (buffers.empty()) {
                condition.wait_for(locker, std::chrono::seconds(flushTimeInterval));
            }
            //超时阶段
            buffers.push_back(std::move(currentBuffer));
            currentBuffer = std::move(replaceBuffer1);
            BufferToWrite.swap(buffers);
            //如果预备buffer被占用
            if (!nextBuffer) {
                nextBuffer = std::move(replaceBuffer2);
            }
        }

```

### 	3.thread_local

​		项目中有两个地方使用到了thread_local，第一个logger对象，保证了每个线程对应一个logger，每个线程能有独立的日志格式和日志名称。第二个时tid，因为项目采用的时RAII方式对一条日志进行输出，而我们得到一个线程id需要sys_call陷入内核态，若一个线程中频繁进入内核态显然是很消耗性能并且没必要。因此我们可将一个线程中tid设置为thread_local以保证每一个线程中有独立的副本，且只会调用一次sys_call。

### 4.template的SFINAE

​		这是模板的一个特性，解释为匹配失败不算错误。项目中利用这个特性结合函数重载，实现了支持多种类型的输出。当然也可引入c++20的约束，但是由于项目保证版本兼容，没有采用20及20以上的特性。

## 项目憧憬

1. 完善更多样化的数据类型
2. 增加输出格式，类似c中printf()的格式化输出
3. 更全面化的用户自定义
4. 增加字体的颜色等特色
5. 进一步调整性能
