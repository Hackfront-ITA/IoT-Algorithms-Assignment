Fs = 200;
filename = argv(){1};

x = importdata(filename);
L = size(x);
L = L(1);

start = 13.4;
si = (start * Fs) + 1;
x = x(si:end);

m = mean(x);
print(m);
