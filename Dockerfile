FROM ubuntu:18.04

# O-RAN upstream repo: release or staging (staging probably
# only makes sense if you also set ORAN_VERSIONS=latest).
ARG ORAN_REPO=release

# O-RAN version-specific package support: set to `latest` to
# get the latest stuff upstream in packagecloud $ORAN_REPO.
ARG ORAN_VERSIONS=
ARG MDCLOG_VERSION=0.1.1-1
ARG RICXFCPP_VERSION=2.3.3
ARG RMR_VERSION=4.4.6

RUN apt-get update \
  && apt-get install -y cmake g++ libssl-dev rapidjson-dev git \
    ca-certificates curl gnupg apt-transport-https apt-utils \
    pkg-config asn1c \
  && curl -s https://packagecloud.io/install/repositories/o-ran-sc/${ORAN_REPO}/script.deb.sh | os=debian dist=stretch bash  \
  && ( [ "${ORAN_VERSIONS}" = "latest" ] \
      || apt-get install -y \
             mdclog=$MDCLOG_VERSION mdclog-dev=$MDCLOG_VERSION \
             ricxfcpp=$RICXFCPP_VERSION ricxfcpp-dev=$RICXFCPP_VERSION \
	     rmr=$RMR_VERSION rmr-dev=$RMR_VERSION \
      && apt-get install -y \
             mdclog mdclog-dev \
	     ricxfcpp ricxfcpp-dev \
	     rmr rmr-dev \
     ) \
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
