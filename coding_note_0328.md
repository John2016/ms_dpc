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
