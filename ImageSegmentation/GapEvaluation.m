clear;

data1 = load('img_mini_rgb.dat');

eva1 = evalclusters(data1, 'kmeans', 'gap', 'KList', [1:10])
figure(1);
plot(eva1);

data2 = load('img_mini_hsb.dat');

eva2 = evalclusters(data2, 'kmeans', 'gap', 'KList', [1:10])
figure(2);
plot(eva2);