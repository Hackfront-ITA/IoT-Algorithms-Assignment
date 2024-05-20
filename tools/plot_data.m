Fs = 200;
filename = argv(){1};
%filename = "/mnt/storage/main/Progetti/Varie/iot-algo-assignment/res/measurements/adaptive_350+420.log";

x = importdata(filename);
L = size(x);
L = L(1);

T = 1 / Fs;
t = (0:L-1) * T;

figure('units', 'normalized', 'outerposition', [0 0 1 1]);
p = plot(t, x);
xlabel('time (s)');
ylabel('power (mW)');

axis([20, 21, 0, 600]);
drawnow;

plotfile = strrep(filename, '.', '_');
plotfile = strcat(plotfile, '.png');
saveas(p, plotfile, 'png');
