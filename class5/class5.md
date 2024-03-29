# 进程通信

```rust
struct mailbox{
    isEmpty:semaphore,
    hasMail:semaphore,
}

pub send(){
    P(isEmpty);
    //...
    V(hasMail);
}

pub receive(){
    P(hasMail);
    /...
    V(isEmpty);
}
```
