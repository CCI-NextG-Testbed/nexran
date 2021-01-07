FROM ubuntu:18.04

RUN apt-get update \
  && apt-get install -y cmake g++ libssl-dev rapidjson-dev git ca-certificates \
  && rm -rf /var/lib/apt/lists/*

RUN cd /tmp \
  && git clone https://gitlab.flux.utah.edu/powderrenewpublic/pistache \
  && cd pistache && mkdir build && cd build \
  && cmake ../ && make install && ldconfig \
  && cd .. && rm -rf /tmp/pistache

COPY . /nexran

RUN cd /nexran && rm -rf build && mkdir build && cd build \
  && cmake -DCMAKE_BUILD_TYPE=Debug ../ && make install && ldconfig

CMD [ "/usr/local/bin/nexran" ]

EXPOSE 8000
