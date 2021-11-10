mod socks;

use log::LevelFilter;
use simple_logger::SimpleLogger;
use std::{net::{TcpListener}, thread};

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
						thread::spawn(move|| {
							socks::socksv5_handle(&mut stream);
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
