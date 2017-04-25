## 20170211 代码修改计划
1. 降低算法空间复杂度
    a  修改dc求解部分，采用池塘采样法求解，减小空间复杂度
     . 在代码中增加空间复杂度预估部分并输出
2. LSH部分的编写
　　　　a. 仿照map reduce，将所有任务分成同等大小的任务块儿，将所有任务分布式执行，不必将数据统一存放

## 计划
1. 第一步，修改dc，尚不采用min hash，跑程序
2. 继续编写基于min hash的程序，完成实验

##　20170213　说明
1. hash版的并行实现，之前有两种思路，划分函数表和划分数据，目前采用数据划分
2. 目前打算将函数和数据均分块后，生成任务列表，按任务划分来实现并行。这样做能充分利用节点的计算能力，但会增加通信开销，且代码编写难度增加
3. 编程思路：
    a. 生成任务列表，存储数据索引、函数参数
    b. 安排几个数据节点，以循环形式接收来自计算节点的数据请求，并回传数据
    c. 计算节点循环请求数据，并计算结果
4. 这里有一个问题，如果单个hash table的大小是1000，那么三个hash值构成的hash tables，最多可能有1G种可能，这有点多
5. 一个问题是如何在低时间复杂度的要求下，将数据转为hash表的形式存储
6. 一个点子，在min hash之后，使用一个随机投影，是否可行（即两层局部敏感哈希）
7. 在计算hash table部分，data patetion应该比assignment partetion更好
8. 循环fetch信息，仍然需要严格控制每一条信息的请求和接收，并没有更灵活

## 20170214　代码笔记
1. 基于lsh的算法必须用池塘采样法确定dc，距离矩阵的计算没有必要
2. 目前的程序的问题是：当文件数量少而数据量多时，能调用的节点少
3. 每个节点对哈希表进行统计后再回传，这时回传的形式是：(group_label,(hash value : (idx)')')

## 20170215　代码
1. 在完成hash table的分析之后，进行每一个hash set内部的diatance, rho计算
2. 任务分配计划：
    for m_groups:
        decrease_sort by hash_set.idxes.size()
        for sorted hash_table.group_hash_table:
            （以一个hash set为单位）依次循环分配任务　＆　记录每一个节点当前的任务负荷
            接收每个任务返回的rho
        整理一个group里的rho
    取每个数据点的最大rho
    广播全局rho
    for m_groups:
        各节点保留之前的数据，并根据rho计算local　delta
        接收各节点的delta　并　整理
    取每个数据点的最小delta
    later process (delta reprovement, decision make and clustering) 

    recv(data size)
    recv(data idxes)
    recv(data)
    distance mat calculation
    rho calculation
    send(rho)
    bcast(global_rho)
    delta calculation
    send(delta)
3. 这里可以考虑采用非阻塞性发送接收
4. 由于并行计算，哈希表中存储的idx是局部的索引值，而回传哈希表时slave节点并不知道前面节点的数据个数，因此在master接收数据时，对索引进行修改
5. 如果后期想要实现负载均衡，应该定期检测各任务的负载量（分配任务时记录各节点负载量即可）
6. 计算节点在完成自己的所有任务后再返回

## 20170216 coding note
1. rho的回传，会涉及到顺序的问题，这里需要谨慎处置。两种思路：
    a. 将每一个hash set单独处理并发送，这里需要
    b. 在每一个计算节点上，处理完所有的hash set，依照它们的idx合并好之后一次发送。这里取第二种
2. 应当注意，在slave节点上，每一个hash item set对应的数据是单独的
3. 需要一个三维数组来存储每一个slave节点上的data mat & distance mat
4. 这时候需要考虑是否将data 和　distance变量设为全局的问题，此时应该将其设为函数内变量　
5. data的问题是可以通过改写函数来实现，但distance不能，因为每一个hash set的大小不一样，导致distance的维度不同
6. 在每一轮分配任务，直接进行计算并返回结果，之后再进行下一个循环的分配（未）

## 20170217 coding plan and notes
1. delta求解过程中的upslope部分需要将idx因素加入考虑
2. 回想起来，前面有一个master节点接收回传数据时，重新计算idx的方法不太对，不存在“加偏移量”的问题
3. slave节点中，其实并不需要rho_global（也许之后delta校正的时候需要）
4. 为了后面delta计算的方便，每个slave节点因该保存自己任务的idx向量，共m组

## 20170220 coding plan
1. 上午已基本完成代码编写，下面开始细化各部分代码，包括
    重新设计不同节点上常用变量的位置
    删除注释／无效代码
    修正代码逻辑
    完善class部分的定义、函数等
2. 新的代码尽量避免全局变量，增加结构体的使用
3. master节点存储一份数据，slave节点只存储被分配到的数据（三维数组形式）
    每个group　×　每个hash item partetion ×　 data / data ,四维数组

## 20170221　coding
1. 将每一个group里面的每一个hash item作为一个class，包含m，idx_item，elements_idx，assign_proc等信息，然后从头开始循环，分配任务；而slave procs可以直接用while接收信号
2. get min hash的函数没有写好，这里要解决一个问题，就是在min hash前，数据的表示问题，是从０开始表示，还是从min value开始表示，暂定从0开始表示吧，这样方便计算

## 2017 0222 coding
1. 在昨天笔记中提到的class，甚至可以加上数据和函数，这样只需找到class传递函数即可，可以提前通过均衡负载分配好任务并记录在obj中
2. 这里要注意，在所有的聚类操作均改为局部变量后，原函数文件中的函数都要对应修改，去除全局变量的影响
3. 代码修正也基本完成，下面的四个任务
    a. dc估计算法
    b. class定义补全，高维vector初始化
    c.　delta 修正算法
    d. 根据丰富类定义的想法写出另一版代码

## 20170312 coding plan
1. 所有代码中，行邻接链表代替邻接矩阵
2. STL中的hashmap类型来存储哈希表
3. 水塘采样实现
4. 完成数据基本信息的统计

## 20170316 coding note
1. 多对一哈希表的实现
    value部分放一个vector或者链表
    将key和value反向存放，利用multimap的性质，但是时间复杂度高，存储要求也许会高
    private Map<Integer, Map<Integer, Map<String, Double>>> cachedMap; 递归式存放
2. ms问题中，用STL:set作为value，这样也方便之后的merge
3. 注意本次修改代码，目的之一是要尽可能较少通信量

## 20170321 coding note
1. 几处疑问
    数据预处理是否需要并行，如果不并行，那将花费大量时间，且浪费资源
    但如果并行的话，因为之后的数据需要重新分配，所以可能需要大量的通信时间
    相关的还有数据分块是否需要并行
2. 犹豫的原因主要是可选策略太多，且无法很容易地评价优劣
    但如果考虑实际情况，例如一个节点的内存无法加载所有的数据，那我们在这一步就必须并行处理（暂且这样吧）
3. 这里的partition部分，top4 & precursor相当于LSH中且的部分，而多个top则相当于或的部分，不同的是这里的或没有划分开，子集允许重叠
4. 另外由于同一个idx最初可能存在于多个不同的key中，因此需使用multimap
5. 自定义类型传输，MPI_Type_vector、MPI_Type_indexed和MPI_Type_struct
6. 散列表结构用multimap存储，能节省查找效率，省去了自己写二叉树的时间，但需要学习multimap的使用
7. 实验表明，上述假设正确，upper_bound等于下一个元素的lower_bound

## 20170322 coding note
1. 非自定义类，如哈希表在mpi中如何传递（其他的还有set等）
2. 在传递多个数据子集时，有两个选项：
    将所有数据子集打包发送（数据结构可能难以构造）
    依子集顺序，顺序发送所有数据（需要非阻塞发送，可能增加通信时间，因为通信次数大量增加）
3. 现在确定一下几次数据传递，按复杂程度排序 
    广播信息，包括单个数据、final_rho等
    slave计算结果，一组计算可以被整合为单个向量（m_group算是多组），但可能有其它附带信息（如idx等），
        这里可以使用多种优化措施，仅传递vector，但master节点需要保存大量参数
        也可以构造并传递结构体，但可能比较复杂
    slave回传的分块结果，可以仅回传哈希值，也可以回传哈希表，后者更复杂
        （这里其实还要考虑计算时间和传递时间的平衡）
    master的任务分配，需要同时传递idx，data，
4. 关于几种通信方式的效率，有篇文章总结道，当发送5000个数据时，连续调用MPI_Send需要9.92s（注意是连续调用同样的通信），而count方法只需要2.24ms，派生方法需要3.18ms，pack方法需要9.20ms，效果十分明显
5. 今天的一个好结果是将用于任务分配的dict和用于计算哈希的hash_map彻底分开，这样只需要设计两个结构体即可
6. 任务分配的结构，其实有两种，一种是<num_proc，idxes>构成的向量，另一种是<num_proc, vector<ms_dataset>>构成的向量，区别在于前者的num_proc是可以重复的，后者是唯一的

## 20170323 coding note
1. error类型重定义，是由于从两个不同的路径包含了同一个头文件而引起的，可在头文件中加入#pragma once解决
2. using namespace std前无分号，可能是因为前面include的某一个文件末尾无分号


## 20170324 coding note
1. 今天重点解决派生类型的传递
    struct
    pack
    Boost.MPI+BoostSerialization
    protobuf 比boost更轻量
2. 最终决定使用protobuf，这是google公司的一个协议
3. Streaming computing models can naturally express fine-grained communication and allow for on-the-fly processing of large data sets with limited memory consumption.
4. 一个概念：streaming computing models
5. protobuf里面可以存放map和unordered_map类型的数据
6. 记住之后要学习boost mpi
7. 所以目前的完整方案：protobuf将class变成string，之后以char的形式发送，最后从char类型重建原数据

## coding plan 0328
1. time clock
2. 日志
3. mpi_probe

## coding plan 0329
1. 完善代码，修改bugs
2. 将hash_map 修改为 unordered_map.c++11以后，不能再使用hash_map，map为红黑树存储，时间效率低，空间效率高，unordered-map相反
	关于map，multimap, unordered_map和unordered_multimap的区别
3. 为了留下hash算法的接口，目前还是采用哈希表实现 
4. 需要注意的是，目前我们的数据划分部分，采用的是多对多的设计
5. 知识：inline friend std::ostream& operator<<(std::ostream& os, Person& p)，定义在class里面，cout重载
6. bool operator==(const Person& p)，定义在class内部，==重载
7. 遇到一个问题，unordered_map里面的hash function不能接收额外参数，因此不同的group，不同的哈希函数都只能用来初始化不同的map对象。
8. 本质上，unordered_map必须包含一个唯一的Hasher，因此不可能出现同一个object中有多个Hasher的情况，因此无论什么解决办法都绕不过这个
9. 使用multimap，红黑树，也比较省空间，可以直接insert另一个map

## coding note 0330
1. protobuff中定义的struct不能和其他文件中的class重名
2. protobuff的版本号，2和3区别较大
3. 在编译使用了protobuf的工程时，会引用很多静态库，这些库默认会被安装到/uer/local/lib中，确保这个目录在环境变量中，即可编译通过

## coding note 0331
1. 今天的主要任务是调试protobuf的bug，如果晚上还没有搞明白，就换boost mpi
2. 如果要删除protobuf的话，需要清下面几个目录：/usr/local,/usr/lib/,/usr/include
3. 最终的解决十分奇葩，只需要在mpicxx之前运行pkg-config，而不是像那条命令一样加在后面，不是版本问题，看来需要多看看cpp的configure和make
4. 上一步并未解决问题，在-o的时候依然报错，之后的解决方法是
    mpicxx *.h *.cc *.cpp -o main -pthread -I/usr/local/include -pthread -L/usr/local/lib -lprotobuf -lpthread，
    后面这些选项使用pkg-config生成的，不知为什么，直接将pkg命令加在mpicxx后不能生效
5. 程序刚刚可以运行时，只跑两个mgf文件，依然会卡死
6. 一种改进方案：fseek，ftell，fread将文件一次性读入内存，之后再操作


## coding note 0401
1. 第一个bug：data.spectra.ions在erase之后，.size()的大小会变化，而在基本数据上的实验没有问题。换了几种方法，发现问题在precursor的读取上。
2. load_data的时候，如何从一个数组中读取任意形式的double，首先简单方法要求数字和其它符号之间必须有空格，切必须明确指明字符串中所有形式的数据和顺序（char，doubl， int等），总体上可行的方法有如下几种
        a. split
        b. regrex
        c. 逐个char读取及分析
        d. sscanf，这个比较适合我们的场景，就是这个
3. 未来计划，在一些class中重载<<操作符
4. 理论上，原数据在assign完成后，master节点即可释放
5. 第二个bug，在分配函数，程序执行两个循环之后，报错，double free or corruption，这样的错误一般出现在释放已经不在的指针的时候 

## coding note 0402
1. 继续昨天的bug，目前可以确定是vector<class>操作的时候，对象销毁不当的问题，现在考虑的解决方案有如下几种：
    a. 严格区分指针和对象操作，学习c++中的一些缺省做法，或者在循环外初始化类指针，在循环内操作并clear
    b. 也可能是用push-back直接操作类内成员造成的不当，考虑使用类内函数来添加数据

## coding note 0403
1. 继续解决昨天的
2. push_back在添加class时，可以直接以构造函数的参数作为参数，效果等同，但未能解决问题
3. 主要问题是vector定义时，push-back时是对象还是指针操作，这里有：rule of three等规则，默认push-back会存储指针而非对象，因此在循环结束时会有double free
4. copy constructor，这个是解决办法

## coding note 0404
1. rule of three(or 4/5), constructor, destructor, copy constructor, assignment opertor
2. 还有一个解决方案是存储为vector<*>，这样只需保证调用时class可用即可

## coding note 0405
1. 昨天的问题基本上用copy constructor解决了
2. 新问题，与protobuf有关,protobuf check failed initial_value != NULL
3. 完成了发送前的数据填充工作
4. 目前可以定位到是传参的错误
5. 传参的几种方法，值传递，指针传递和引用传递，&放置的位置不同意义也不同，引用一个vector时，可以vector<>&，或者声明iterator，取*iterator,引用一个对象时，可以用值传递和地址传递（形参写*，调用写&）,最终用地址传递解决了这一问题
6. 新bug，mpi_recv的错误，其实是char数组初始化的错误，char tmp[size]直接从栈中分配空间，而栈的空间有限，可能只有1,000,000，因此应该用malloc从堆中分配空间
7. protobuf会生成值为0或负数的“字符”，因此不能用简单的string构造函数来构造string，另外MPI相关函数在处理字符数组时，不关心char的值
8. 一个情况是，小数据情况下，大多数数据集都只包含一个数据，这时需要优化graph和rho的求解，保证正确和高效
9. 在分散数据集情况下，会有许多数据upslope被置为-1，当存在众多单数据数据集时尤其如此，在当前工作中，由于存在大量的类，故将所有upslope为-1的都置为新类

## coding note 0406
1. 天河系统学习
2. shell脚本学习
3. 进一步测试并优化代码
4. 天河机安装protobuf

## coding note 0419
1. 在得到delta之后，程序会无法运行，需要修复 ———— 多写一个bcast，导致程序卡死，这个错误编译过程检测不到

## coding note 0420
1. 程序中对于rho和delta的处理似乎都不太对，重新整理，第一个问题是最终的输出未加分隔符，导致输出错误
2. 改正之后，发现输出的this->rho和delta都是nan，那聚类过程一定是错的
3. 在本机上，文件流打开后，输出显示流无法正常输出
4. bug:在归一化rho和delta过程中，除数为零导致nan

## coding note 0422
1. 昨天的程序由于后台运行，没有写输入参数，导致运行失败，现在尝试输入重定向

## coding note 0423
1. 分析result_csv，查看其中的rho和delta情况，来重新决定decision making的方法：
    不是decision的问题，而是rho和delta的求解方法的问题，甚至是距离求解方法的问题（dc可能不合理）
2. 调整代码，一次性加载整个数据文件来提升效率
3. 调整代码，使得单个节点能够处理更多的数据文件，或将多个节点用来发送数据
4. 需要注意的是，PRIDE2迭代式聚类，最终未给出similarity threshold，这个算是他们的实验结果，不是预先设定的；另外值得注意的是，窗口的大小也是随着聚类过程不断扩大的，这也体现了tolerance的增加，从0.2增加到4（这里是指precursor？）。这里给出一个方案：调高dc，看看结果，之前的dc太低了

## coding note 0425
1. 将哈希表写入文件，包括precursor， top mz，size
2. 注意，hashed table unique只与mz值得global范围有关，大致为<br>(precursor_range/pre_tolerance)*(mzvalue_rangez/mz_talerance)<br>这也只是一个理论情况，可以认为是上限，数据的增多应该主要影响precursor_range的数量
3. 经过计算，哪怕仅有一个峰匹配，那两个spectra的相似度也会大于0.03