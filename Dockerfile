FROM davidfrantz/base:latest as builder

# disable interactive frontends
ENV DEBIAN_FRONTEND=noninteractive 

# Environment variables
ENV SOURCE_DIR $HOME/src/fire-spread
ENV INSTALL_DIR $HOME/bin

# Copy src to SOURCE_DIR
RUN mkdir -p $SOURCE_DIR
WORKDIR $SOURCE_DIR
COPY --chown=docker:docker . .

# Build, install
RUN echo "building fire-spread" && \
  make

#FROM davidfrantz/fire-spread:latest as fire-spread

#COPY --chown=docker:docker --from=builder $HOME/bin $HOME/bin

WORKDIR /home/docker

CMD ["fire-spread"]
