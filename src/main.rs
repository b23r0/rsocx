mod socks;

use log::LevelFilter;
use simple_logger::SimpleLogger;

use async_std::{io, net::{TcpListener}, prelude::*, task};

fn usage() {
    return;
}


#[async_std::main]
async fn main() -> io::Result<()>  {
	SimpleLogger::new().with_colors(true).init().unwrap();
	::log::set_max_level(LevelFilter::Info);

	let arg_count = std::env::args().count();

	if  arg_count == 1{
		usage();
	}

	let first  = std::env::args().nth(1).expect("parameter not enough");

	match first.as_str() {
		"-l" => {
			let port = std::env::args().nth(2).unwrap();
			log::info!("listen to : {}" , "0.0.0.0:".to_string() + &port);
			
			let listener = TcpListener::bind("0.0.0.0:".to_string() + &port).await?;

			let mut incoming = listener.incoming();

			while let Some(stream) = incoming.next().await {
				let stream = stream?;
				task::spawn(async {
					socks::socksv5_handle(stream).await;
				});
			}
		},
		_ => {
			usage();
		}
	}
	Ok(())
}
