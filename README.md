# rsocx [![Build Status](https://img.shields.io/github/workflow/status/b23r0/rsocx/Rust)](https://github.com/b23r0/rsocx/actions/workflows/rust.yml) [![ChatOnDiscord](https://img.shields.io/badge/chat-on%20discord-blue)](https://discord.gg/ZKtYMvDFN4) [![Crate](https://img.shields.io/crates/v/rsocx)](https://crates.io/crates/rsocx)
A high performence Socks5 proxy server with bind/reverse support implementation by Rust 

# Features

* Async-std
* No unsafe code
* Single executable
* Linux/Windows/Mac/BSD support
* Support reverse mode(Not bind any port in client)

# Build & Run

`$> cargo build --release`

# Installation

`$> cargo install rsocx`

# Usage

## Bind Mode

You can run a socks5 proxy and listen port at 1080

`$> ./rsocx -l 0.0.0.0:1080`

## Reverse Mode

First listen a port waiting for slave connection

`$> ./rsocx -t 0.0.0.0:8000 -s 0.0.0.0:1080`

then reverse connect to master in slave

`$> ./cliws -r 127.0.0.1:8000`

# Benchmark

Simple load test through `proxychains4` visit to Tornado's `helloworld` case in LAN.


```python
import grequests
import time
start = time.time()
req_list = [grequests.get('http://192.168.0.222:8888') for i in range(1000)]
res_list = grequests.map(req_list)
print(time.time()-start)
```

## Test Envoriment

| Envoriment    | Value           |
|-------------- |-----------      |
| Proxy OS      | Windows11       |
| CPU           | i7-9700k        |
| Target OS     | Ubuntu20.04     |
| Network       | LAN             |
| Target Server | [Tornado(Python)](https://github.com/tornadoweb/tornado/blob/master/demos/helloworld/helloworld.py) |
| Test Count    | 1k              |
| Socks5 client | Proxychains4    |

## Test Result

| Project        | Language | Base        | Take Time |
|----------------|----------|-------------|-----------|
| rsocx          | Rust     | Async-std   | 12.90s    |
| rsocx(reverse) | Rust     | Aysnc-std   | 24.65s    |
| [merino](https://github.com/ajmwagar/merino)         | Rust     | Tokio       | 12.37s    |
| [go-socks5](https://github.com/armon/go-socks5)      | Golang   | goroutine   | 12.31s    |
| [simple-socks](https://github.com/brozeph/simple-socks)   | Nodejs   | async       | 13.71s     |
| [asio5](https://github.com/liuaifu/asio5)          | C++      | Boost::Asio | 12.37s    |
| [esocks](https://github.com/fengyouchao/esocks)          | Java      | Thread-Pool | 25.06s    |

(Test Date : 13 Nov 2021)

# Socks5 Protocol Support

- [x] IPV6 Support
- [ ] `SOCKS5` Authentication Methods
  - [x] `NOAUTH` 
  - [ ] `USERPASS`
- [ ] `SOCKS5` Commands
  - [x] `CONNECT`
  - [ ] `BIND`
  - [ ] `ASSOCIATE` 

# Reference

* https://github.com/ajmwagar/merino

* https://github.com/ylxdzsw/v2socks
