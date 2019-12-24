function CVRubiksCube()
    run("ExampleSet", 0.25);
end

function run(setN, t)
    n = 15;
    f = 182.5;
    m = 150;
    flag = 1;
    mse = 100000;
    
    disp("Starting run...");
    
    for l = 1:6
        fileName = "Images/" + setN + "/Img" + l + ".jpg";
        I = imread(fileName);
        [preRotI, rotI, lines] = detectEdges(I, t, n, f, m);
        [max_len,lens,thetas,rhos,x1s,y1s,x2s,y2s] = plotEdges(rotI, lines, l, flag);
        [c1, c2, c3, c4, mse] = detectCorners(l, max_len, lens, thetas, rhos, x1s, y1s, x2s, y2s);
        if (mse >= 10000)
            break;
        end
        findROIs(I, preRotI, c1, c2, c3, c4, l);
    end
    
    if (mse < 10000)
   
        red = [180 20 25];
        green = [4 75 61];
        blue = [12 32 90];
        orange = [230 120 26];
        yellow = [215 200 9];
        white = [180 180 180];
        
        colors1 = [];
        colors2 = [];
        colors3 = [];
        colors4 = [];
        colors5 = [];
        colors6 = [];

        for l = 1:6
            fileName = "CroppedImages/Img" + l + ".jpg";
            I = imread(fileName);
            color = splitIntoStickers(I, l, red, green, blue, orange, yellow, white);
             if (l == 1)
                colors1 = [colors1 color];
             elseif (l == 2)
                colors2 = [colors2 color];
             elseif (l == 3)
                colors3 = [colors3 color];
             elseif (l == 4)
                colors4 = [colors4 color];
             elseif (l == 5)
                colors5 = [colors5 color];
             else
                colors6 = [colors6 color];
             end
        end

        colorsCube = [colors1 ; colors2 ; colors3 ; colors4 ; colors5 ; colors6];

        fileID = fopen("stickers.txt",'w');
        fprintf(fileID,'%c %c %c %c %c %c %c %c %c\n', colorsCube.');
        fclose(fileID);
        disp("Program ran successfully!");
    else
        disp("Cant fit square! MSE of " + mse + " is too large!");
    end
    disp(" ");
end

function [I1GBT, rotI, lines] = detectEdges(I1, threshold, numPeaks, fillGap, minLength)
    I1G = rgb2gray(I1);
    I1GB = medfilt2(I1G);
    I1GBT = imbinarize(I1GB,threshold);   
    rotI = imrotate(I1GBT, 33, 'crop');
    
    BW = edge(rotI, 'prewitt');
    [H,T,R] = hough(BW);
    P = houghpeaks(H,numPeaks,'threshold',ceil(0.3*max(H(:))));
    lines = houghlines(BW,T,R,P,'FillGap',fillGap,'MinLength',minLength);
end

function [max_len,lens,thetas,rhos,x1s,y1s,x2s,y2s] = plotEdges(I, lines, n, flag)
    lens = double.empty;
    thetas = double.empty;
    rhos = double.empty;
    x1s = double.empty;
    y1s = double.empty;
    x2s = double.empty;
    y2s = double.empty;
    
    subplot(2,3,n), imshow(I), title("Image " + int2str(n)), hold on
    max_len = 0;
    xy_long = [0,0;0,0];
    for k = 1:length(lines)
       if (~((lines(k).theta == 57 && (lines(k).rho <= 580 || lines(k).rho >= 2020)) ...
          || (lines(k).theta == -33 && (lines(k).rho <= -599 || lines(k).rho >= 1961))))
      
           if ((lines(k).point1(1) > 230) && (lines(k).point2(1) < 2330))
               xy = [lines(k).point1; lines(k).point2];
               plot(xy(:,1),xy(:,2),'LineWidth',2,'Color','green');

               % Plot beginnings and ends of lines
               plot(xy(1,1),xy(1,2),'x','LineWidth',2,'Color','yellow');
               plot(xy(2,1),xy(2,2),'x','LineWidth',2,'Color','red');

               % Determine the endpoints of the longest line segment
               len = norm(lines(k).point1 - lines(k).point2);
               if ( len > max_len)
                  max_len = len;
                  xy_long = xy;
               end

               lens(end+1) = len;
               thetas(end+1) = lines(k).theta;
               rhos(end+1) = lines(k).rho;
               x1s(end+1) = xy(1,1);
               y1s(end+1) = xy(1,2);
               x2s(end+1) = xy(2,1);
               y2s(end+1) = xy(2,2);
           end
       end
    end
    plot(xy_long(:,1),xy_long(:,2),'LineWidth',2,'Color','cyan');
    
    if (flag == 1)
        close;
    end
end

function [c1,c2,c3,c4,mse] = detectCorners(n,max_len,lens,thetas,rhos,x1s,y1s,x2s,y2s)
    minx = 10000;
    miny = 10000;
    maxx = 0;
    maxy = 0;
    
    c1 = [0 0];
    c2 = [0 0];
    c3 = [0 0];
    c4 = [0 0];
    
    for k = 1:length(lens) 
        if (x1s(k) < minx)
            minx = x1s(k);
            c1 = [x1s(k) y1s(k)];
        end
        
        if (x2s(k) < minx)
            minx = x2s(k);
            c1 = [x2s(k) y2s(k)];
        end
        
        if (y1s(k) < miny)
            miny = y1s(k);
            c2 = [x1s(k) y1s(k)];
        end
        
        if (y2s(k) < miny)
            miny = y2s(k);
            c2 = [x2s(k) y2s(k)];
        end
        
        if (x1s(k) > maxx)
            maxx = x1s(k);
            c3 = [x1s(k) y1s(k)];
        end
        
        if (x2s(k) > maxx)
            maxx = x2s(k);
            c3 = [x2s(k) y2s(k)];
        end
        
        if (y1s(k) > maxy)
            maxy = y1s(k);
            c4 = [x1s(k) y1s(k)];
        end
        
        if (y2s(k) > maxy)
            maxy = y2s(k);
            c4 = [x2s(k) y2s(k)];
        end
    end

    dc1c2 = norm(c1 - c2);
    dc2c3 = norm(c2 - c3);
    dc3c4 = norm(c3 - c4);
    dc4c1 = norm(c4 - c1);
    
    errl1 = abs(max_len - dc1c2);
    errl2 = abs(max_len - dc2c3);
    errl3 = abs(max_len - dc3c4);
    errl4 = abs(max_len - dc4c1);
    
    mse = (errl1^2  + errl2^2 + errl3^2 + errl4^2);
end

function findROIs(I, preRotI, c1, c2, c3, c4, n)
    [rows, columns, numberOfColorChannels] = size(I);
    Xc = columns / 2;
    Yc = rows / 2;
    
    X = [c1(1) c2(1) c3(1) c4(1)];
    Y = [c1(2) c2(2) c3(2) c4(2)];
    
    ang = -33;
    Xrot =  (X-Xc)*cosd(ang) + (Y-Yc)*sind(ang) + Xc;
    Yrot = -(X-Xc)*sind(ang) + (Y-Yc)*cosd(ang) + Yc;
    
    Xstart = min(Xrot);
    Ystart = min(Yrot);
    width = max(Xrot) - Xstart;
    height = max(Yrot) - Ystart;
	Icropped = imcrop(I, [Xstart Ystart width height]);
    
	iptsetpref('ImshowBorder','tight');
    figure, imshow(Icropped);
    saveas(gcf, "CroppedImages/Img" + int2str(n) + ".jpg");
    close;
end

 
function colorsForThisFace = splitIntoStickers(I,n, red, green, blue, orange, yellow, white)
    [rows, columns, numberOfColorChannels] = size(I);
    Xs = 0;
    Ys = 0;
    w = ceil((columns - Xs)/3);
    h = ceil((rows - Ys)/3);
    off = 60;
    
    colorsForThisFace = ['' '' '' '' '' '' '' '' ''];
   
    s = 1;
    for l = 0:2
        for m = 0:2
            Il = imcrop(I, [Xs+m*w+off Ys+l*h+off w-2*off h-2*off]);
            colorsForThisFace(s) = detectColor(Il, red, green, blue, orange, yellow, white);
            s = s + 1;
        end
    end 
end

function color = detectColor(I, red, green, blue, orange, yellow, white)
    IB = medfilt3(I);   
    [rows, columns, depth] = size(IB);
    
    redCh = uint8.empty;
    greenCh = uint8.empty;
    blueCh = uint8.empty;
    
    for v = 1:50
        x = round(1 + (columns-1).*rand(1,1));
        y = round(1 + (rows-1).*rand(1,1));

        redVal = IB(y, x, 1);
        greenVal = IB(y, x, 2);
        blueVal = IB(y, x, 3);

        if (redVal + greenVal + blueVal >= 80)
            redCh(end+1) = redVal;
            greenCh(end+1) = greenVal;
            blueCh(end+1) = blueVal;
        end
    end
    
    medRed = double(median(redCh));
    medGreen = double(median(greenCh));
    medBlue = double(median(blueCh));
  
    mseRed = (red(1) - medRed)^2 + (red(2) - medGreen)^2 + (red(3) - medBlue)^2;
    mseGreen = (green(1) - medRed)^2 + (green(2) - medGreen)^2 + (green(3) - medBlue)^2;
    mseBlue = (blue(1) - medRed)^2 + (blue(2) - medGreen)^2 + (blue(3) - medBlue)^2;
    mseOrange = (orange(1) - medRed)^2 + (orange(2) - medGreen)^2 + (orange(3) - medBlue)^2;
    mseYellow = (yellow(1) - medRed)^2 + (yellow(2) - medGreen)^2 + (yellow(3) - medBlue)^2;
    mseWhite = (white(1) - medRed)^2 + (white(2) - medGreen)^2 + (white(3) - medBlue)^2;
       
    pred = min([mseRed mseGreen mseBlue mseOrange mseYellow mseWhite]);
       
    if (pred == mseRed)
        color = 'R';
    elseif (pred == mseGreen)
        color = 'G';
    elseif (pred == mseBlue)
        color = 'B';
    elseif (pred == mseOrange)
        color = 'O';
    elseif (pred == mseYellow)
        color = 'Y';
    else
        color = 'W';
    end
end
