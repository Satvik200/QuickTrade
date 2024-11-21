echo "Building QUICKTRADE...."

echo "Generating orders.  Please wait..."
perl OrderGenerator.pl > orders.txt
echo "orders generated successfully!"

echo "Building Release. Please wait..."
make all
echo "Release built successfully!"

echo "Running build..."
./QuickTradeProcessor orders.txt