import subprocess

# 输入字体文件
font_file = "agave.ttf"
# 输出子集字体文件
output_file = "aaa.ttf"
# 字符清单文件（里面写常用字和符号）
chars_file = "char.txt"

# 调用 pyftsubset
subprocess.run([
    "pyftsubset", font_file,
    f"--text-file={chars_file}",
    f"--output-file={output_file}", 
])
