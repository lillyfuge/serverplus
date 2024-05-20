​

 C++20带来了许多新特性。在深入了解四大核心特性之前，这里先概述一下C++20的一些基本情况。除了四大主要特性之外，C++20还在语言核心、库以及并发能力等方面引入了许多改进和新功能。

### **编译器的支持**

编译器对C++20的支持情况 通过实践新特性是熟悉它们的最简单方法。那么，自然而然就会产生一个问题：哪些编译器支持C++20的哪些特性呢？通常情况下cppreference.com/compiler_support这个网站能为你提供关于语言核心和库的支持情况的答案。

简单来说，最新版本的GCC、Clang以及EDG编译器对C++20语言核心提供了最佳的支持。此外，微软的MSVC编译器和Apple的Clang编译器也支持了许多C++20的新特性。

![](https://img-blog.csdnimg.cn/direct/615e0418cc084358adc362be703b4c6d.png)![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")​编辑

### 一  concepts

   泛型编程使用模板的关键思想是定义能够适用于多种类型的函数和类。然而，经常会出现使用了错误类型实例化模板的情况，其结果往往是好几页难以理解的错误信息。这一令人沮丧的情形随着“概念”的引入而终结。

   “概念”使你能够为模板编写需求条件，这些条件可由编译器进行检查。概念彻底改变了我们思考和编写泛型代码的方式。原因如下：

1. **模板的需求成为接口的一部分**：明确地表达了模板应当工作的类型需要满足的条件。
2. **基于概念的重载与特化**：函数的重载或类模板的特化可以根据概念来进行，使得选择更精确、意图更清晰。
3. **改进的错误消息**：因为编译器可以比较模板参数的需求与实际提供的模板实参，当不匹配时能给出更有针对性的错误提示。

但这还不是故事的全部：

- **预定义概念与自定义概念**：你可以使用预定义好的概念，也可以根据需要自定义概念来满足特定需求。
- **auto与概念的统一使用**：概念的引入使得在某些场景下可以替代`auto`关键字，使得类型推导更加智能且带有约束条件。
- **自动模板化函数声明**：如果一个函数声明中使用了概念，该函数会自动成为一个函数模板，极大地简化了函数模板的编写过程，使之几乎与编写普通函数一样直接。

因此，概念不仅提升了代码的可读性和编写效率，还显著改善了编译时错误的诊断体验，是C++20中对泛型编程领域的一大革新。

例：

```cpp
template<typename T>
concept Integral = std::is_integral<T>::value;

Integral auto gcd(Integral auto a,                       Integral auto b){
    if( b == 0 ) return a; 
    else return gcd(b, a % b);
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

Integral是一个概念，它要求类型参数T满足std::is_integral<T>::value为真。std::is_integral<T>::value是来自类型特征库的一个函数，用于在编译时检查类型T是否为整数类型。如果std::is_integral<T>::value评估为true，则一切正常；如果不是，将得到一个编译时错误。对于那些好奇的人（你应该保持好奇心），我有关于类型特征库的帖子可以参考。

gcd算法基于欧几里得算法来确定最大公约数。我使用了所谓的简化函数模板语法来定义gcd。gcd要求其参数和返回类型都支持Integral这个概念。gcd是一个函数模板，对它的参数和返回值都设定了要求。如果去掉语法糖，你或许能更清晰地看到gcd的基本本质。

```cpp
template<typename T>
requires Integral<T>()
T gcd(T a, T b){
    if( b == 0 ) return a; 
    else return gcd(b, a % b);
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

### 二 Ranges Library

范围库（Ranges Library）是概念（Concepts）的第一个重要应用领域。它支持的算法具有以下特点：

1. **直接作用于容器**：无需通过迭代器来指定操作范围，可以直接在容器上执行操作。
2. **惰性求值**：算法可以在需要结果时才进行计算，这有助于提高效率，特别是在处理大量数据时。
3. **可组合性**：算法可以像函数一样轻松地组合在一起，形成更复杂的数据处理流程。

简而言之，范围库促进了函数式编程模式的应用，使得C++编程在处理集合和序列数据时更加灵活高效，同时也更加简洁易读。

```cpp
#include <vector>
#include <ranges>
#include <iostream>
int main(){
  std::vector<int> ints{0, 1, 2, 3, 4, 5};
  auto even = [](int i){ return 0 == i % 2; };
  auto square = [](int i) { return i * i; };

  for (int i : ints | std::view::filter(even) | 
                      std::view::transform(square)) {
    std::cout << i << ' ';             // 0 4 16
  }
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

even是一个lambda函数，用于判断给定的整数i是否为偶数；另一个lambda函数square则将输入的i映射到它的平方。其余部分则是函数组合的概念，你需要从左至右阅读这段代码：for (int i : ints | std::view::filter(even) | std::view::transform(square))。这段代码的意思是对ints中的每个元素应用过滤器even，然后将剩下的每个元素通过square映射到其平方。如果你熟悉函数式编程，这段代码读起来就像普通的叙述一样流畅。

### 三 Coroutines

   协程（Coroutines）是一种可以被挂起和恢复执行并且能保持其内部状态的一般化函数。它们通常是编写事件驱动型应用程序的标准方式。这类应用程序包括模拟、游戏、服务器、用户界面，甚至是算法。协程也被广泛用于协同多任务处理，即任务主动放弃控制权，允许其他任务运行，之后再恢复执行。

   C++20并未直接提供具体的协程实现，而是提供了一个框架，让你能够根据需求编写自己的协程。这个编写协程的框架包含超过20个函数，你需要部分实现它们，或者在某些情况下重写它们。这样的设计使得协程能够高度定制化，以适应不同的应用场景和需求。通过这些工具，开发者能够更灵活地控制程序的执行流，提升程序的响应性和性能。

```cpp
Generator<int> getNext(int start = 0, int step = 1){
    auto value = start;
    for (int i = 0;; ++i){
        co_yield value;            // 1
        value += step;
    }
}

int main() {

    std::cout << std::endl;

    std::cout << "getNext():";
    auto gen = getNext();
    for (int i = 0; i <= 10; ++i) {
        gen.next();               // 2
        std::cout << " " << gen.getValue();                  
    }

    std::cout << "\n\n";

    std::cout << "getNext(100, -10):";
    auto gen2 = getNext(100, -10);
    for (int i = 0; i <= 20; ++i) {
        gen2.next();             // 3
        std::cout << " " << gen2.getValue();
    }

    std::cout << std::endl;

}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

函数getNext是一个协程，因为它使用了关键词co_yield。

getNext包含一个无限循环，每次循环在co_yield之后返回一个值。

通过对next()的调用（第2行和第3行）会恢复协程的执行，随后的getValue()调用则获取该值。在getNext调用之后，协程会再次暂停，直到下一次next()调用才继续。在我的示例中有一个大大的未知数，**那就是getNext函数的返回类型Generator<int>**。这里就开始涉及到一些复杂的内容，这些内容将会在关于协程的详细文章中进行讲解。

### 四 Modules

模块承诺带来以下好处：

- 缩短编译时间
- 宏隔离，减少宏污染
- 表达代码的逻辑结构
- 使头文件变得多余
- 摆脱繁琐且不雅观的宏解决方案

​
