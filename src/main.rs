mod utils;
mod socks;

use log::LevelFilter;
use simple_logger::SimpleLogger;
use getopts::Options;
use tokio::{io::{self, AsyncWriteExt, AsyncReadExt}, task, net::{TcpListener, TcpStream}};
use utils::MAGIC_FLAG;

fn usage(program: &str, opts: &Options) {
    let program_path = std::path::PathBuf::from(program);
    let program_name = program_path.file_stem().unwrap().to_str().unwrap();
    let brief = format!("Usage: {} [-ltr] [IP_ADDRESS] [-s] [IP_ADDRESS]",
                        program_name);
    print!("{}", opts.usage(&brief));
}


#[tokio::main]
async fn main() -> io::Result<()>  {
	SimpleLogger::new().with_utc_timestamps().with_utc_timestamps().with_colors(true).init().unwrap();
	::log::set_max_level(LevelFilter::Info);

    let args: Vec<String> = std::env::args().collect();
    let program = args[0].clone();

    let mut opts = Options::new();

    opts.optopt("l",
                "bind",
                "The address on which to listen socks5 server for incoming requests",
                "BIND_ADDR");

	opts.optopt("t",
                "transfer",
                "The address accept from slave socks5 server connection",
                "TRANSFER_ADDRESS");

	opts.optopt("r",
                "reverse",
                "reverse socks5 server connect to master",
                "TRANSFER_ADDRESS");

	opts.optopt("s",
                "server",
                "The address on which to listen local socks5 server",
                "TRANSFER_ADDRESS");

	let matches = opts.parse(&args[1..]).unwrap_or_else(|_| {
		usage(&program, &opts);
		std::process::exit(-1);
	});

	if matches.opt_count("l") > 0{
		let local_address : String = match match matches.opt_str("l"){
			Some(p) => p,
			None => {
				log::error!("not found listen port . eg : rsocx -l 0.0.0.0:8000");
				return Ok(());
			},
		}.parse(){
			Err(_) => {
				log::error!("not found listen port . eg : rsocx -l 0.0.0.0:8000");
				return Ok(());
			},
			Ok(p) => p
		};
		log::info!("listen to : {}" ,local_address);
		
		let listener = match TcpListener::bind(&local_address).await{
			Err(e) => {
				log::error!("error : {}", e);
				return Ok(());
			},
			Ok(p) => p
		};

		loop{
			let (stream , _) = listener.accept().await.unwrap();
			tokio::spawn(async {
				socks::socksv5_handle(stream).await;
			});
		}
	} else if matches.opt_count("t") > 0 {
		let master_addr : String = match match matches.opt_str("t"){
			Some(p) => p,
			None => {
				log::error!("not found listen port . eg : rsocx -t 0.0.0.0:8000 -s 0.0.0.0:1080");
				return Ok(());
			},
		}.parse(){
			Err(_) => {
				log::error!("not found listen port . eg : rsocx -t 0.0.0.0:8000 -s 0.0.0.0:1080");
				return Ok(());
			},
			Ok(p) => p
		};
		let socks_addr : String = match match matches.opt_str("s"){
			Some(p) => p,
			None => {
				log::error!("not found listen port . eg : rsocx -t 0.0.0.0:8000 -s 0.0.0.0:1080");
				return Ok(());
			},
		}.parse(){
			Err(_) => {
				log::error!("not found listen port . eg : rsocx -t 0.0.0.0:8000 -s 0.0.0.0:1080");
				return Ok(());
			},
			Ok(p) => p
		};

		log::info!("listen to : {} waiting for slave" , master_addr);
		
		let slave_listener = match TcpListener::bind(&master_addr).await{
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

		log::info!("listen to : {}" , socks_addr);
		
		let listener = match TcpListener::bind(&socks_addr).await{
			Err(e) => {
				log::error!("error : {}", e);
				return Ok(());
			},
			Ok(p) => p
		};

		loop {
			let (mut stream , _) = listener.accept().await.unwrap();

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
					tokio::select! {
						a = proxy_stream.read(&mut buf1) => {
		
							let len = match a {
								Err(_) => {
									break;
								}
								Ok(p) => p
							};
							match stream.write_all(&mut buf1[..len]).await {
								Err(_) => {
									break;
								}
								Ok(p) => p
							};
		
							if len == 0 {
								break;
							}
						},
						b = stream.read(&mut buf2) =>  { 
							let len = match b{
								Err(_) => {
									break;
								}
								Ok(p) => p
							};
							match proxy_stream.write_all(&mut buf2[..len]).await {
								Err(_) => {
									break;
								}
								Ok(p) => p
							};
							if len == 0 {
								break;
							}
						},
					}
				}
				log::info!("transfer [{}:{}] finished" , slave_addr.ip() , slave_addr.port());
			});
		}
	}else if matches.opt_count("r") > 0{
		let fulladdr : String = match match matches.opt_str("r"){
			Some(p) => p,
			None => {
				log::error!("not found ip . eg : rsocx -r 192.168.0.1:8000");
				return Ok(());
			},
		}.parse(){
			Err(_) => {
				log::error!("not found ip . eg : rsocx -r 192.168.0.1:8000");
				return Ok(());
			},
			Ok(p) => p
		};
		
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

	} else {
		usage(&program, &opts);
	}
	Ok(())
}
