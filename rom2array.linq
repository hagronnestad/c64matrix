<Query Kind="Statements" />

var f = File.ReadAllBytes(@"C:\dev\c64-font2char\FontToChar\FontToChar\bin\Debug\net5.0-windows\matrix.rom");

"const unsigned char MATRIX_CODE_CHARS[4096] = {".Dump();

for (int i = 0; i < f.Length; i += 16)
{
	Debug.Write($"    0x{f[i+0]:X2}, 0x{f[i+1]:X2}, 0x{f[i+2]:X2}, 0x{f[i+3]:X2}, 0x{f[i+4]:X2}, 0x{f[i+5]:X2}, 0x{f[i+6]:X2}, 0x{f[i+7]:X2}, 0x{f[i+8]:X2}, 0x{f[i+9]:X2}, 0x{f[i+10]:X2}, 0x{f[i+11]:X2}, 0x{f[i+12]:X2}, 0x{f[i+13]:X2}, 0x{f[i+14]:X2}, 0x{f[i+15]:X2},\n");
}

"};".Dump();