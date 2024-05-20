​

C++20带来了四大特性，这些特性改变了我们思考和编写现代C++的方式：概念（Concepts）、范围库（Ranges）、协程（Coroutines）以及模块（Modules）。我已经针对概念和范围库写过一些文章。现在，让我们更深入地了解一下协程。

协程是一种能够保存状态并在需要时暂停及恢复执行的函数。在C++中，函数的发展又迈出了新的一步。虽然我将协程作为C++20的新概念介绍，但实际上这一思想相当古老。Melvin Conway首次提出了“协程”这一术语，并在1963年关于编译器构造的出版物中使用了它。Donald Knuth则进一步指出，过程（procedures）可以视为协程的一个特例。

C++20通过引入co_await和co_yield这两个新关键字，扩展了C++函数的执行方式，引入了两种新概念。借助co_await 表达式，可以实现表达式的挂起与恢复执行。如果在一个函数func中使用了co_await 表达式，当调用auto getResult = func()且函数结果不可用时，并不会造成阻塞。相比资源消耗较大的阻塞操作，协程提供了资源友好的等待方式。而co_yield 表达式则使得编写生成器函数成为可能。生成器函数每次调用时都会返回一个新的值，它像一个数据流，从中可以按需提取值，这个数据流甚至可以是无限的。因此，协程处于惰性求值的核心位置。

```cpp
int func1() {
    return 1972;
}
int func2(int arg) {
    return arg;
}
double func2(double arg) {
    return arg;
}
template <typename T>
T func3(T arg) {
    return arg;
}
struct FuncObject4 {
    int operator()() { // (1)
        return 1998;
    }
};
auto func5 = [] {
    return 2011;
};
auto func6 = [] (auto arg){
    return arg;
};
int main() {
    func1();        // 1972
    func2(1998);    // 1998
    func2(1998.0);  // 1998.0
    func3(1998);    // 1998
    func3(1998.0);  // 1998.0
    FuncObject4 func4;
    func4();        // 1998
    func5();        // 2011
    func6(2014);    // 2014
    func6(2014.0);  // 2014
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

自从1972年首个C语言标准问世以来，我们就有了基本的函数形式：`func1`。

到了1998年首个C++标准发布时，函数的功能变得更加强大，我们获得了：

函数重载（Function Overloading）：允许我们定义多个同名函数，但参数列表不同，如func2。

函数模板（Function Templates）：通过模板参数，使函数能够适用于多种数据类型，例如func3

函数对象（Function Objects，也常被称为仿函数Functors）：这些是重载了调用运算符operator()的对象，使得它们可以像函数一样被调用，如func4。第一对圆括号代表对象本身，第二对圆括号则代表函数调用的参数。

C++11引入了Lambda函数（Lambda Expressions），这是一种方便创建匿名函数对象的方式，如func5。

C++14中，Lambda函数更进一步，支持了泛型Lambda，即Lambda内部可以使用自动类型推导的变量，这增加了Lambda的灵活性，例如`func6`。

现在，让我们更进一步探索协程领域中的特殊成员——生成器（Generators）。生成器是一种特殊的协程，其主要目的是按需生成一系列值而不是一次性计算所有结果，从而实现了惰性计算。通过`co_yield`，生成器能够在每次调用时产出一个新的值，且能够记住当前的状态，以便下一次从相同位置继续执行。这种方式对于处理大量数据流或是无限序列特别有效，因为它避免了预先计算整个序列的开销。

Generators

在传统的C++中，我可以实现一个贪婪的生成器（Greedy Generator）。

**贪婪生成器示例**

下面的程序尽可能地直接明了。函数getNumbers返回从begin到end的所有整数，每次增加inc。其中，begin必须小于end，而且inc必须是正数。

```cpp
#include <iostream>
#include <vector>
std::vector<int> getNumbers(int begin, int end, int inc = 1) {
    std::vector<int> numbers;                      // (1)
    for (int i = begin; i < end; i += inc) {
        numbers.push_back(i);
    }
    return numbers;
}
int main() {
    std::cout << std::endl;
    const auto numbers= getNumbers(-10, 11);
    for (auto n: numbers) std::cout << n << " ";
    std::cout << "\n\n";
    for (auto n: getNumbers(0, 101, 5)) std::cout << n << " ";
    std::cout << "\n\n";
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

对程序的两点观察至关重要。一方面，第(1)行中的向量数字总是获取所有值。即使我只对一个含有1000个元素的向量的前五个元素感兴趣，也是如此。另一方面，将函数getNumbers转换为惰性生成器是相当容易的。

```cpp
#include <iostream>
#include <vector>
generator<int> generatorForNumbers(int begin, int inc = 1) {
  for (int i = begin;; i += inc) {
    co_yield i;
  }
}
int main() {
    std::cout << std::endl;
    const auto numbers= generatorForNumbers(-10);                   // (2)
    for (int i= 1; i <= 20; ++i) std::cout << numbers << " ";       // (4)
    std::cout << "\n\n";                              
    for (auto n: generatorForNumbers(0, 5)) std::cout << n << " ";  // (3)
    std::cout << "\n\n";
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

auto可以返回一个函数指针（直接运行）

在greedyGenerator.cpp文件中，函数getNumbers返回一个std::vector，而lazyGenerator.cpp中的协程generatorForNumbers则返回一个生成器。

第(2)行中的生成器numbers或第(3)行中的generatorForNumbers(0, 5)会在请求时产生一个新的数字。基于范围的for循环触发了这些请求。更确切地说，协程的每次请求通过co_yield i返回值i并立即暂停其执行。当需要新值时，协程会从该处精确地恢复执行。

第(3)行中的表达式generatorForNumbers(0, 5)是对生成器的一种即时使用方式。我想特别强调一点：协程generatorForNumbers创建了一个无限数据流，因为第(3)行中的for循环没有结束条件。如果我只是请求有限数量的值，比如第(4)行所示，那么这个无限数据流是没有问题的。然而，对于第(3)行来说，由于没有结束条件，这种情况并不成立。因此，该表达式会无限运行下去。

​
