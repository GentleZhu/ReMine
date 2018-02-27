make
mkdir -p tools/tokenizer/build/
javac -cp ".:tools/tokenizer/lib/*" tools/tokenizer/src/Tokenizer.java -d tools/tokenizer/build/