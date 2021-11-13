pub fn makeword(a : u8, b : u8) -> u16{
	return ((a as u16) << 8) | b as u16;
}
/*
pub fn splitword(a : u16) -> (u8 , u8) {
    return ((a >> 8) as u8  , a as u8)
}*/