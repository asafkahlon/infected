FROM ubuntu:16.04

ENV INFECTED_HOME /code/infected

RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y build-essential
RUN apt-get install -y libcunit1 libcunit1-doc libcunit1-dev

RUN mkdir -p ${INFECTED_HOME}

COPY src ${INFECTED_HOME}/src
COPY include ${INFECTED_HOME}/include

ENTRYPOINT ["/usr/bin/make", "-C", "/code/infected/src", "clean", "tests"]
