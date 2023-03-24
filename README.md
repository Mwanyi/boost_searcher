## 基于Boost的搜索引擎

### 项目简介

这是一个基于Boost的站内搜索引擎，通过输入关键字可以实现该关键字在Boost官网内的搜索，可以点击标题跳转到官网对应界面

### 技术栈和环境

- 技术栈：C/C++，C++11，STL，准标准库boost，Jsoncpp，cppjieba，cpp-httplib
- 环境：Centos 7服务器，vim，gcc，Makefile，vscode

### 文件结构

boost_searcher

| -- data

​		| -- input 未处理的boost网页的数据源

​			-- untag_html

​				| -- untag.txt 存储去标签后的文档内容，每一个网页以【标题\3内容\3URL】的方式连接

  -- debug.cpp 用于测试debug用

  -- dir_tool 用到的库文件

​      | -- boost_1_81_0  下载的boost官网库，用于提取数据源

​			 cpp-httplib-v0.7.15 httplib库，可以用于建立服务端

​			 cppjieba 用于分词的库

​			  demo.cpp 分词库cppjieba的使用示范

  -- http_server.cpp http服务器连接

  -- index.hpp 建立正排索引和倒排索引

| -- log 存储日志信息

​	   | -- log.txt 存储日志信息

  -- log.hpp 日志头文件

  -- Makefile

  -- nohup.out

  -- parser.cpp 对原始数据进行去标签和数据清洗工作

  -- searcher.hpp 搜索，在索引中搜索关键词对应的文档编号，根据权重倒排进行显示

  -- util.hpp 工具类头文件

   -- wwwroot 用于前端界面设置
	  |-- index.html
`	  -- nohup.out

### 运行方法

- 进入boost_searcher文件后，先进行编译

  ```shell
  make
  ```

- 运行parser进行去标签和数据清洗操作

  ```shell
  ./parser
  ```

- 启动服务器，这里我设置使用8081号端口，开始构建索引，该部分会较慢

  ```shell
  ./http_server
  ```

  此时可以另开一个服务器查看是否已经开启

  ```shell
  netstat -nltp
  ```

- 开启服务器后，可在网页输入【本机ip号:端口号】访问网页

- 即可在页面输入关键字进行查找，并且点击标题可以跳转到Boost对应官网

