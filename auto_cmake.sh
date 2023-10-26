# 自动编译并运行测试的linux脚本
export PROGRAMNAME=gramma
cd build/
cmake ..
make
./${PROGRAMNAME} -fuck
cd ..