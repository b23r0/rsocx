# rsocx [![Build Status](https://app.travis-ci.com/b23r0/rsocx.svg?branch=main)](https://app.travis-ci.com/b23r0/rsocx) [![ChatOnDiscord](https://img.shields.io/badge/chat-on%20discord-blue)](https://discord.gg/ZKtYMvDFN4)
A high performence Socks5 proxy with bind/reverse support implementation by Rust

# Features

* Async-std
* No unsafe code
* Single executable
* Linux/Windows/Mac/BSD support
* Support reverse mode(Not bind any port in client)

# Build & Run

`$> cargo build --release`

`$> ./target/release/rsocx`

# Usage

## Bind Mode

You can run a socks5 proxy and listen port at 1080

`$> ./rsocx -l 1080`

## Reverse Mode

First listen a port waiting for slave connection

`$> ./rsocx -l 8000 1080`

then reverse connect to master in slave

`$> ./cliws -r 127.0.0.1 8000`

# Reference

* https://github.com/ajmwagar/merino

* https://github.com/kost/revsocks
