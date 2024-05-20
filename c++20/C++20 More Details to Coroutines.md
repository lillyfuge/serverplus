​

在我上一篇文章（C++20：协程——初览）中向您介绍了我对协程的第一印象后，今天我想提供更多的细节。同样，C++20带给我们的不是协程本身，而是一个用于构建协程的框架。

我在这篇及后续文章中的任务是解释构建协程的框架。最终，您可以创建自己的协程，或者使用现有的协程实现，比如Lewis Baker开发的优秀的cppcoro库。

今天的帖子处于中间地带：既不是对协程的概述，也不是接下来几篇文章中将深入探讨的协程框架。您可能首先会问的问题是：我应该在什么时候使用协程？

### 典型应用场景

协程通常是编写事件驱动型应用程序的标准方式。这类事件驱动型应用可以是模拟、游戏、服务器、用户界面或算法。例如，几年前我用Python编写了一个除颤器模拟器。这个模拟器特别帮助我们进行了临床可用性研究。除颤器是一个事件驱动型应用，因此我自然而然地基于事件驱动的Python框架twisted来实现了它。

协程也常被用于协作式多任务处理。协作式多任务处理的关键在于，每个任务可以按需占用时间。这与抢占式多任务处理形成对比，在抢占式多任务处理中，是由调度器决定每个任务能占用CPU多久。协作式多任务处理常常使并发变得更加简单，因为并发任务在关键区域不会被中断。如果你对“协作式”和“抢占式”这两个术语还感到困惑，我找到了一篇很好的概述文章，你可以在这里阅读：协作式与抢占式：最大化并发能力的探索。

### Underlying Ideas

C++20中的协程是不对称的、第一类的且无栈的。不对称协程的工作流程会返回到调用者处。 第一类协程的表现就如同数据一样。像数据一样行为意味着可以将它们作为函数的参数，或从函数返回值，或将它们存储在变量中。 无栈协程允许暂停和恢复顶级协程的执行。协程的执行及其产出结果会回到调用者那里。相比之下，有栈协程会在Windows上预留1MB，默认情况下在Linux上预留2MB的栈空间用于存储局部变量和函数调用信息。

设**计目标** Gor Nishanov，负责C++中协程标准化的工作，阐述了协程的设计目标。协程应当：

1. **高度可扩展**（至数十亿个并发协程）。
2. **拥有高效的成本**：挂起和恢复操作的开销应与函数调用的开销相当。
3. **无缝交互**：与现有设施无痛集成，没有额外开销。
4. **开放式协程机制**：允许库设计师开发协程库，具有高度灵活性。
5. **支持多种高级语义**：如生成器、goroutine、任务等。
6. **适用性广泛**：即使在禁止或不支持异常的环境中也能使用。

这些设计目标旨在确保协程成为现代软件开发中强大且灵活的并发工具，能够适应不同的应用场景和性能需求，同时保持语言的简洁性和兼容性。

### Becoming a Coroutine

一个函数中一旦使用了关键字 co_return、co_yield 或 co_await，便会隐式地成为一个协程。

**co_return:** 协程使用 co_return 作为返回语句。它用于从协程中返回一个值，并结束当前协程的执行，同时允许调用者获取该返回值。

**co_yield:** 通过 co_yield，你可以实现一个生成器，该生成器能生成一个无限数据流，从中可以相继查询值。比如在上一篇文章（C++20: 协程 - 初探）中的函数 generatorForNumbers(int begin, int inc = 1) 的返回类型就是一个生成器。生成器内部持有一个特殊的承诺对象 pro，这样 co_yield i 就等同于调用 co_await pro.yield_value(i)。紧接着这个调用之后，协程就会被暂停。

**co_await:** co_await 最终导致协程的执行被暂停或恢复。表达式 co_await exp 中的 exp 必须是一个所谓的“可等待”表达式。exp 需要实现一个特定的接口。这个接口包含三个函数：await_ready、await_suspend 和 await_resume。这三个函数共同控制着如何准备等待、挂起协程以及在恢复时做什么操作。

### Two Awaitables

C++20 标准已经定义了两个基本的构建块作为“可等待”类型：std::suspend_always 和 std::suspend_never。

std::suspend_always

```cpp
struct suspend_always {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

std::suspend_never

```cpp
struct suspend_never {
    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

### A Blocking and a Waiting Server

```cpp
Acceptor acceptor{443};               // (1)
while (true){
    Socket socket= acceptor.accept(); // blocking (2)
    auto request= socket.read();      // blocking (3)
    auto response= handleRequest(request);
    socket.write(response);           // blocking (4)
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

顺序服务器在同一线程中回答每个请求。它在端口 443 上监听（第 1 行），接受连接（第 2 行），从客户端读取传入数据（第 3 行），并将响应写回客户端（第 4 行）。第 2、3 和 4 行中的调用是阻塞的，意味着在这些操作完成之前，程序不会继续执行后续代码。

借助于 `co_await`，原本阻塞的调用现在可以被挂起和恢复。这样一来，资源密集型的阻塞服务器转变成了节省资源的等待服务器。这意味着服务器在等待某个操作（如接受连接、读取数据或写入响应）完成期间，不会占用线程资源，可以释放出来处理其他任务，从而提高了整体效率和响应能力。

```cpp
Acceptor acceptor{443};

while (true){
    Socket socket= co_await acceptor.accept();
    auto request= co_await socket.read();
    auto response= handleRequest(request);
    co_await socket.write(response);
}
```

![](data:image/gif;base64,R0lGODlhAQABAPABAP///wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw== "点击并拖拽以移动")

你可能已经猜到，理解这个协程的关键部分在于`co_await expr`调用中的“可等待”表达式`expr`。这些表达式必须实现`await_ready`、`await_suspend`和`await_resume`这三个函数。这些函数共同定义了当`co_await`表达式被执行时的行为：

await_ready：检查操作是否可以直接完成，不需要挂起协程。如果返回true，表示操作已就绪，无需挂起；反之，则协程将被挂起。

await_suspend：当操作未就绪需要挂起协程时调用，它接收一个代表协程悬置状态的句柄，并决定是否真的需要挂起操作。这允许进行资源管理或进一步的调度决策。

await_resume：当协程从挂起状态恢复执行时调用，用于恢复操作并通常返回一个结果给协程，以继续其执行流程。

**接下来是什么？** 编写协程的框架包含了超过20个函数，你需要部分实现它们，或者在某些情况下重写它们。我的下一篇帖子将会更深入地探讨这个框架，带你深入了解如何实现和自定义这些函数，以满足特定的应用需求和优化并发性能。我们将探讨这些核心函数的作用、如何正确实现它们以及如何利用它们来构建高效、灵活的并发逻辑。

​
