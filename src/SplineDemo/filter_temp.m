numElements = 2;     % Number of mics
spacing     = 0.145; % Distance of mics
speedSound  = 368.88 % Speed of Sound

ANGLE_RESOLUTION=180;    % Number of angle ranges that beamformer should go through them % This means the beams moves from zero to 180 by 1 degree difference. so the spatial resolution of the beams are only 1 degree.
startAngle_index=0*(ANGLE_RESOLUTION/180); % the prepherial angles zero to 10 degrees and 170 to 180 degrees are ignored. It means that beams dont sweep these two area. This resolution in space is done to increase the speed of the algorithm
endAngle_index=180*(ANGLE_RESOLUTION/180);

% SteeringAngle is composed of angles from 10 to 170 degree by resolution of 1 degree
SteeringAngle =  180.0 * (startAngle_index:endAngle_index) / (ANGLE_RESOLUTION-1);
SteeringAngle=SteeringAngle*pi/180;%SteeringAngle in degree
LS=length(SteeringAngle); %number of beams

% initializing some vectors to record the results in beam sweeping iteration 
logOutput=zeros(numchans,LS);
Output_Index=zeros(numchans,LS);
Index_MAX_P_L=zeros(numchans,1);

%making the delay(time) and sample_delay vector for beamforming
delay = spacing * (-cos(SteeringAngle)) / speedSound; %to compensate for the delay in the left and right channel
sample_delay=round(delay*SamplingFre);

%%beamformer; sweeping beams from 10 to 170 degree for each filter channel(numchans times).
for band=1:numchans

    for beam=1:LS %Steer the band (Ls is the number of beams)

        % these two if conditions avoid repetitive computation 
        % for beams with the same sample_delay and reduce the calculation time
        if beam > 1 
            
            if sample_delay(beam) ~= sample_delay(beam-1)

                %the summation process of the left and right channel 
                if (sample_delay(beam) > 0)
                    OutSum = Fframe{1}(:,band) + [ zeros(abs(sample_delay(beam)), 1) ; Fframe{2}(1:end-abs(sample_delay(beam)), band) ];
                
                else
                    OutSum = Fframe{1}(:,band) + [ Fframe{2}(abs(sample_delay(beam))+1:end,band) ; zeros(abs(sample_delay(beam)),1) ];
                
                end

                %normalize the output signal of beamformer by number
                %of channels
                OutSum = OutSum / numElements;

                %storing the peak of the all beams 
                Abs_OutSum=abs(OutSum);                  
                MAX_OutSum=max(Abs_OutSum);
                logOutput(band,beam) = MAX_OutSum; %band is the filterbank index, beam is the index of spatial beams

            else
                % if the sample delay is the same just store the same
                % previous output for the current band

                Abs_OutSum=abs(OutSum);
                MAX_OutSum=max(Abs_OutSum);
                logOutput(band,beam) = (MAX_OutSum);

            end
        
        else
            % For the first band (beam==1), initialization of the process
            if (sample_delay(beam) > 0)
                OutSum=Fframe{1}(:,band) + [ zeros(abs(sample_delay(beam)), 1) ; Fframe{2}(1:end - abs(sample_delay(beam)), band)];

            else
                OutSum=Fframe{1}(:,band) + [ Fframe{2}(abs(sample_delay(beam)) + 1:end, band) ; zeros(abs(sample_delay(beam)), 1)];

            end

            %normalize the output signal of beamformer by number of channels                
            OutSum = OutSum / numElements;
            [MAX_OutSum, ~]=max(abs(OutSum));


            logOutput(band, beam) = (MAX_OutSum);%band is the filterbank index, beam is the index of spatial beams

        end %beam>1
    end%SteeringAngle
end %num of bins(filters)