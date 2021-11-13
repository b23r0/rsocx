include!("utils.rs");
mod socks;

use log::LevelFilter;
use simple_logger::SimpleLogger;

use futures::{AsyncReadExt, AsyncWriteExt, FutureExt, StreamExt};
use async_std::{io, net::{TcpListener, TcpStream}, task};
use futures::select;

pub static MAGIC_FLAG : [u8;2] = [0x37, 0x37];

fn usage() {
	println!("rsocx - A high performence Socks5 proxy with bind/reverse support implementation by Rust");
	println!("https://github.com/b23r0/rsocx");
	println!("Usage: rsocx [-l socks5 port] [-t [slave_port] [socks5 port]] [-r [master ip] [master port]]");
}


#[async_std::main]
async fn main() -> io::Result<()>  {
	SimpleLogger::new().with_colors(true).init().unwrap();
	::log::set_max_level(LevelFilter::Info);

	let arg_count = std::env::args().count();

	if  arg_count == 1{
		usage();
		return Ok(());
	}

	let first  = std::env::args().nth(1).unwrap();

	match first.as_str() {
		"-l" => {
			let port = match std::env::args().nth(2){
				None => {
					log::error!("not found listen port . eg : rsocx -l 8000");
					return Ok(());
				},
				Some(p) => p
			};
			log::info!("listen to : {}" , "0.0.0.0:".to_string() + &port);
			
			let listener = match TcpListener::bind("0.0.0.0:".to_string() + &port).await{
				Err(e) => {
					log::error!("error : {}", e);
					return Ok(());
				},
				Ok(p) => p
			};

			let mut incoming = listener.incoming();

			while let Some(stream) = incoming.next().await {
				let stream = stream?;
				task::spawn(async {
					socks::socksv5_handle(stream).await;
				});
			}
		},
		"-t" => {
			let master_port = match std::env::args().nth(2){
				None => {
					log::error!("not found listen port . eg : rsocx -t 9000 8000");
					return Ok(());
				},
				Some(p) => p
			};
			let socks_port = match std::env::args().nth(3){
				None => {
					log::error!("not found listen port . eg : rsocx -t 9000 8000");
					return Ok(());
				},
				Some(p) => p
			};

			log::info!("listen to : {} waiting for slave" , "0.0.0.0:".to_string() + &master_port);
			
			let slave_listener = match TcpListener::bind("0.0.0.0:".to_string() + &master_port).await{
				Err(e) => {
					log::error!("error : {}", e);
					return Ok(());
				},
				Ok(p) => p
			};

			let (mut slave_stream , slave_addr) = match slave_listener.accept().await{
				Err(e) => {
					log::error!("error : {}", e);
					return Ok(());
				},
				Ok(p) => p
			};

			log::info!("accept slave from : {}:{}" , slave_addr.ip() , slave_addr.port() );

			log::info!("listen to : {}" , "0.0.0.0:".to_string() + &socks_port);
			
			let listener = match TcpListener::bind("0.0.0.0:".to_string() + &socks_port).await{
				Err(e) => {
					log::error!("error : {}", e);
					return Ok(());
				},
				Ok(p) => p
			};

			let mut incoming = listener.incoming();

			while let Some(stream) = incoming.next().await {
				let mut stream = stream?;

				match slave_stream.write_all(&[MAGIC_FLAG[0]]).await{
					Err(e) => {
						log::error!("error : {}" , e);
						break;
					}
					_ => {}
				};

				let (mut proxy_stream , slave_addr) = match slave_listener.accept().await{
					Err(e) => {
						log::error!("error : {}", e);
						return Ok(());
					},
					Ok(p) => p
				};

				log::info!("accept from slave : {}:{}" , slave_addr.ip() , slave_addr.port() );

				task::spawn(async move {
					let mut buf1 = [0u8 ; 1024];
					let mut buf2 = [0u8 ; 1024];

					loop{
						select! {
							a = proxy_stream.read(&mut buf1).fuse() => {
			
								let len = match a {
									Err(e) => {
										log::error!("error : {}" , e);
										break;
									}
									Ok(p) => p
								};
								match stream.write_all(&mut buf1[..len]).await {
									Err(e) => {
										log::error!("error : {}" , e);
										break;
									}
									Ok(p) => p
								};
			
								if len == 0 {
									break;
								}
							},
							b = stream.read(&mut buf2).fuse() =>  { 
								let len = match b{
									Err(e) => {
										log::error!("error : {}" , e);
										break;
									}
									Ok(p) => p
								};
								match proxy_stream.write_all(&mut buf2[..len]).await {
									Err(e) => {
										log::error!("error : {}" , e);
										break;
									}
									Ok(p) => p
								};
								if len == 0 {
									break;
								}
							},
							complete => break,
						}
					}
				});
			}
		},
		"-r" => {
			let addr = match std::env::args().nth(2){
				None => {
					log::error!("not found ip . eg : rsocx -r 192.168.0.1 9000");
					return Ok(());
				},
				Some(p) => p
			};
			let port = match std::env::args().nth(3){
				None => {
					log::error!("not found port . eg : rsocx -r 192.168.0.1 9000");
					return Ok(());
				},
				Some(p) => p
			};
			let fulladdr = format!("{}:{}" , addr , port);
			let mut master_stream = match TcpStream::connect(fulladdr.clone()).await{
				Err(e) => {
					log::error!("error : {}", e);
					return Ok(());
				},
				Ok(p) => p
			};
			log::info!("connect to {} success" ,fulladdr );
			loop {
				let mut buf = [0u8 ; 1];
				match master_stream.read_exact(&mut buf).await{
					Err(e) => {
						log::error!("error : {}", e);
						return Ok(());
					},
					Ok(p) => p
				};
	
				if buf[0] == MAGIC_FLAG[0] {
					let stream = match TcpStream::connect(fulladdr.clone()).await{
						Err(e) => {
							log::error!("error : {}", e);
							return Ok(());
						},
						Ok(p) => p
					};

					task::spawn(async {
						socks::socksv5_handle(stream).await;
					});
				}
			}

		},
		_ => {
			usage();
		}
	}
	Ok(())
}
