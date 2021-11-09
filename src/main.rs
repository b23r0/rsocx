include!("utils.rs");
#[cfg(target_os = "linux")]
use std::os::unix::prelude::{AsRawFd};
#[cfg(target_os = "linux")]
use errno::{Errno, errno};
#[cfg(target_os = "linux")]
use libc::*;
#[cfg(target_os = "windows")]
use winapi::um::winsock2::{fd_set, select};
#[cfg(target_os = "windows")]
use std::os::windows::prelude::AsRawSocket;
use log::LevelFilter;
use simple_logger::SimpleLogger;

use std::{io::{self, Read, Write}, net::{Ipv6Addr, Shutdown, SocketAddrV6, TcpListener, TcpStream}, ptr, thread};

#[cfg(target_os = "windows")]
unsafe fn FD_ISSET(a : u64 , set : &mut fd_set) -> bool{
	let mut i = 0 ;
	while i < set.fd_count {
		
		if set.fd_array[i as usize] == a as usize {
			return true;
		}

		i += 1;
	}
	return false
}

#[cfg(target_os = "windows")]
unsafe fn FD_SET(a : u64 , set : &mut fd_set){
    use winapi::um::winsock2::FD_SETSIZE;

	let mut i  = 0 ;
	while i < set.fd_count {
		
		if set.fd_array[i as usize] == a as usize {
			return
		}
		i += 1;
	}
	if (i as usize) < FD_SETSIZE {
		set.fd_array[i as usize] = a as usize;
		set.fd_count += 1;
	}
}

#[cfg(target_os = "windows")]
unsafe fn FD_ZERO(set : &mut fd_set){
	let mut i = 0 ;
	while i < set.fd_count {
		set.fd_array[i as usize] =0;
		i += 1;
	}
	set.fd_count = 0;
}

#[derive(Debug, Clone)]
pub enum Addr {
    V4([u8; 4]),
    V6([u8; 16]),
    Domain(Box<[u8]>)
}


fn format_ip_addr(addr :& Addr) -> String {
	match addr {
		Addr::V4(addr) => {
			format!("{}.{}.{}.{}" , addr[0], addr[1] ,addr[2], addr[3])
		},
		Addr::V6(addr) => {
			format!("{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}" , addr[0], addr[1] ,addr[2], addr[3], addr[4], addr[5] ,addr[6], addr[7] , addr[8], addr[9] ,addr[10], addr[11], addr[12], addr[13] ,addr[14], addr[15])
		},
		Addr::Domain(addr) => match String::from_utf8(addr.to_vec()) {
			Ok(p) => p ,
			Err(e) => {
				log::error!("parse domain faild. {}" , e);
				"".to_string()
			},
		}
	}
}

fn usage() {
    return;
}

fn main() {
	SimpleLogger::new().with_colors(true).init().unwrap();
	::log::set_max_level(LevelFilter::Info);

	let arg_count = std::env::args().count();

	if  arg_count == 1{
		usage();
		return;
	}

	let first  = std::env::args().nth(1).expect("parameter not enough");

	match first.as_str() {
		"-l" => {

			let port = match std::env::args().nth(2) {
				None => {
					log::error!("not found listen port . eg : rsocx -l 1080");
					return;
				},
				Some(p) => p
			};
			log::info!("listen to : {}" , "0.0.0.0:".to_string() + &port);
			
			let listener = match TcpListener::bind("0.0.0.0:".to_string() + &port) {
				Err(_) => {
					log::error!("bind port[0.0.0.0:{}] faild" , port);
					return
				},
				Ok(p) => p
			};

			for stream in listener.incoming() {
				match stream {
					Ok(mut stream) => {
						log::info!("accept from: {}", stream.peer_addr().unwrap());

						let mut is_ok = false;

						thread::spawn(move|| {
							loop {
								let mut header = [0u8 ; 2];
								match stream.read_exact(&mut header) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									_ => {}
								};
								
								if header[0] != 5 {
									log::error!("not support protocol version {}" , header[0]);
									break;
								}
							
								let mut methods = vec![0u8; header[1] as usize].into_boxed_slice();
								match stream.read_exact(&mut methods) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									_ => {}
								};
							
								if !methods.contains(&0u8) {
									log::error!("just support no auth");
									break;
								}
							
								match stream.write_all(&[5, 0]){
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
            						_ => {}
								};

								let mut request =  [0u8; 4];
								match stream.read_exact(&mut request){
									Err(e) => {
										log::error!("error: {}" , e);
										break;
									},
									_ => {}
								}

								if request[0] != 5 {
									log::error!("say again not support version: {}" , request[0]);
									break;
								}
							
								if request[1] != 1 {
									log::error!("not support cmd: {}" , request[0]);
									break;
								}
							
								let addr = match request[3] {
									0x01 => {
										let mut ipv4 =  [0u8; 4];
										match stream.read_exact(&mut ipv4){
											Err(e) => {
												log::error!("error: {}" , e);
												break;
											},
											_ => {}
										}
										Addr::V4(ipv4)
									},
									0x04 => {
										let mut ipv6 =  [0u8; 16];
										match stream.read_exact(&mut ipv6){
											Err(e) => {
												log::error!("error: {}" , e);
												break;
											},
											_ => {}
										}
										Addr::V6(ipv6)
									},
									0x03 => {
										let mut domain_size =  [0u8; 1];
										match stream.read_exact(&mut domain_size){
											Err(e) => {
												log::error!("error: {}" , e);
												break;
											},
											_ => {}
										}
										let mut domain =  vec![0u8; domain_size[0] as usize].into_boxed_slice();
										match stream.read_exact(&mut domain){
											Err(e) => {
												log::error!("error: {}" , e);
												break;
											},
											_ => {}
										}

										Addr::Domain(domain)
									},
									_ => {
											log::error!("unknow atyp {}" , request[3]);
											break;
									}
								};
							
								let mut port = [0u8 ; 2];
								match stream.read_exact(&mut port) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									_ => {}
								};

								let port = (port[0] as u16) << 8 | port[1] as u16;
								let address = format!("{}:{}" , format_ip_addr(&addr) , port);

								log::info!("proxy connect to {}:{}" , format_ip_addr(&addr) , port);
								let client : io::Result<TcpStream>;
								match addr{
									Addr::V4(_) => {
										
										client = TcpStream::connect(address.clone());
									},
									Addr::V6(x) => {
										let ipv6 = Ipv6Addr::new(
											makeword(x[0] , x[1]) , 
											makeword(x[2] , x[3]) , 
											makeword(x[4] , x[5]) , 
											makeword(x[6] , x[7])  , 
											makeword(x[8] , x[9]) , 
											makeword(x[10] , x[11]) , 
											makeword(x[12] , x[13]) , 
											makeword(x[14] , x[15])
										);
										let v6sock = SocketAddrV6::new(ipv6 , port , 0 , 0 );
										client = TcpStream::connect(v6sock);
									},
            						Addr::Domain(_) => {
										client = TcpStream::connect(address.clone());
									}
								};

								let mut client = match client {
									Err(_) => {
										log::warn!("connect[{}] faild" , address);
										break;
									},
									Ok(p) => p									
								};

								let remote_port = client.local_addr().unwrap().port();

								let mut reply = Vec::with_capacity(22);
								reply.extend_from_slice(&[5, 0, 0]);
							
								match addr {
									Addr::V4(x) => {
										reply.push(1);
										reply.extend_from_slice(&x);
									},
									Addr::V6(x) => {
										reply.push(4);
										reply.extend_from_slice(&x);
									},
									Addr::Domain(x) => {
										reply.push(3);
										reply.push(x.len() as u8);
										reply.extend_from_slice(&x);
									}
								}
							
								reply.push((remote_port >> 8) as u8);
								reply.push(remote_port as u8);
							
								match stream.write_all(&reply) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									Ok(_) => {}
								};

								match stream.set_nonblocking(true) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									Ok(_) => {}
								};

								match client.set_nonblocking(true) {
									Err(e) => {
										log::info!("error: {}", e);
										break;
									},
									Ok(_) => {}
								};

								#[cfg(target_os = "linux")]
								let fd1 = stream.as_raw_fd();
								#[cfg(target_os = "linux")]
								let fd2  = client.as_raw_fd();

								#[cfg(target_os = "windows")]
								let fd1 = stream.as_raw_socket();
								#[cfg(target_os = "windows")]
								let fd2  = client.as_raw_socket();
								

								let mut rd : fd_set = unsafe { std::mem::zeroed() };

								loop{

									unsafe { FD_ZERO(&mut rd) };
									unsafe { FD_SET(fd1, &mut rd) };
									unsafe { FD_SET(fd2, &mut rd) };

									let res = unsafe { select(
										#[cfg(target_os = "linux")]
										if fd1 > fd2 { fd1 + 1 } else { fd2 + 1} , 
										#[cfg(target_os = "windows")]
										0,
										&mut rd , 
										ptr::null_mut() , 
										ptr::null_mut() , 
										ptr::null_mut()) };

									if res < 0 {
										#[cfg(target_os = "linux")]
										if errno() == Errno(EINTR){
											continue
										}
										break;
									}

									if unsafe { FD_ISSET(fd1, &mut rd) } {
										let mut buf = [0u8 ; 1024];
										let size = match stream.read(&mut buf){
											Err(e) => {
												log::info!("error: {}", e);
												break;
											},
											Ok(p) => p
										};
										
										if size != 0 {
											match client.write_all(&mut buf[..size]){
												Err(e) => {
													log::info!("error: {}", e);
													break;
												},
												Ok(p) => p
											};
										}
									}

									if unsafe { FD_ISSET(fd2, &mut rd) } {
										let mut buf = [0u8 ; 1024];
										let size = match client.read(&mut buf){
											Err(e) => {
												log::info!("error: {}", e);
												break;
											},
											Ok(p) => p
										};
										
										if size != 0 {
											match stream.write_all(&mut buf[..size]){
												Err(e) => {
													log::info!("error: {}", e);
													break;
												},
												Ok(p) => p
											};
										}
									}

								}



								is_ok = true;
								break;
							}
							if !is_ok {
								match stream.shutdown(Shutdown::Both) {
									Err(_) => {},
									_ => {}
								};
							}
							
						});
					}
					Err(e) => {
						log::info!("Error: {}", e);
					}
				}
			}

			return;
		},
		_ => {
			usage();
			return;
		}
	}
}