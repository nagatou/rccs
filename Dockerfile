FROM debian:wheezy
MAINTAINER PRESYSTEMS Inc. <promotion@presystems.xyz>
RUN groupadd -r rccsuser && useradd -r -m -g rccsuser -d /home/rccsuser rccsuser
RUN apt-get update && apt-get install -y git libglib2.0-dev gcc make vim

WORKDIR /home/rccsuser
RUN git clone https://github.com/nagatou/rccs
RUN cd rccs/src \
    && mkdir -p /usr/lib/glib-2.0 && ln -s `pkg-config --cflags glib-2.0 | awk '{print substr($2,3)}'` /usr/lib/glib-2.0 \
    && make rccs \
    && cp rccs /usr/local/bin \
    && chown -fR rccsuser:rccsuser /home/rccsuser 
RUN find / -perm +6000 -type -f -exec chmod a-s {} \; || true
ENV HOME /home/rccsuser
USER rccsuser
ENTRYPOINT ["/bin/bash"]
