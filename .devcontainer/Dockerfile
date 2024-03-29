# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.134.0/containers/cpp/.devcontainer/base.Dockerfile
ARG VARIANT="bullseye"

FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

RUN curl -fsSL https://download.docker.com/linux/debian/gpg | sudo apt-key add - \
  && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - \
  && echo "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye main" >> /etc/apt/sources.list \
  && echo "deb-src http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye main" >> /etc/apt/sources.list \
  && echo "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-12 main"  >> /etc/apt/sources.list \
  && echo "deb-src http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-12 main" >> /etc/apt/sources.list \
  && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg \
  && sudo mv bazel.gpg /etc/apt/trusted.gpg.d/ \
  && echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list \
  && curl -fsSL https://packages.redis.io/gpg | sudo gpg --dearmor -o /usr/share/keyrings/redis-archive-keyring.gpg \
  && echo "deb [signed-by=/usr/share/keyrings/redis-archive-keyring.gpg] https://packages.redis.io/deb $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/redis.list \
  && apt-get update \
  && apt install -y software-properties-common \
  && add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/debian $(lsb_release -cs) stable" \
  && apt-get update \
  && export DEBIAN_FRONTEND=noninteractive \
  && apt-get -y install --no-install-recommends \
  apt-transport-https \
  ca-certificates \
  ninja-build \
  libssl-dev \
  bazel
# && rm -rf /var/lib/apt/lists/* \
# && apt-get clean

RUN curl -L -O https://github.com/gabime/spdlog/archive/refs/tags/v1.9.2.tar.gz \
  && tar xvfz v1.9.2.tar.gz \
  && cd spdlog-1.9.2 && mkdir build && cd build \
  && cmake -G "Ninja" .. \
  && ninja install \
  && cd ../.. \
  && rm -rf spdlog-1.9.2

RUN curl -L -O https://golang.org/dl/go1.15.5.linux-amd64.tar.gz \
  && tar -C /usr/local -xzf go1.15.5.linux-amd64.tar.gz \
  && rm go1.15.5.linux-amd64.tar.gz

RUN curl -L -O https://github.com/bazelbuild/buildtools/releases/download/3.5.0/buildifier \
  && chmod +x buildifier \
  && mv buildifier /usr/local/bin

RUN curl -L -O https://github.com/actor-framework/actor-framework/archive/c2be26e7f9e3e84ef14730590fc4e7b25fb9d29a.zip \
  && unzip -d caf c2be26e7f9e3e84ef14730590fc4e7b25fb9d29a.zip \
  && cd caf/actor-framework-c2be26e7f9e3e84ef14730590fc4e7b25fb9d29a/ \
  && ./configure --generator=Ninja \
  && cd build \
  && cmake -G "Ninja" .. \
  && ninja install \
  && cd ../../.. \
  && rm -rf caf

RUN sudo apt install -y \
  yamllint \
  curl \
  gnupg2 \
  vim \
  graphviz xdot \
  clang-format \
  docker-ce-cli \
  tree \
  moreutils \
  telnet \
  rlwrap \
  iputils-ping \
  ssh-askpass \
  npm \
  tmux \
  libeigen3-dev \
  libtool-bin

RUN apt install -y libzmq3-dev \
  && git clone https://github.com/zeromq/libzmq.git \
  && cd libzmq \
  && ./autogen.sh \
  && ./configure \
  && make \
  && make install 

RUN git clone https://github.com/zeromq/cppzmq.git \
  && cd cppzmq \
  && mkdir build \
  && cd build \
  && cmake .. \
  && make -j4 install

RUN locale-gen en_US.UTF-8
ENV LANG='en_US.UTF-8' LANGUAGE='en_US:en' LC_ALL='en_US.UTF-8'

RUN groupadd docker \
  && usermod -aG docker vscode

USER vscode
WORKDIR /home/vscode

RUN /usr/local/go/bin/go get github.com/bazelbuild/buildtools/buildifier >> /home/vscode/results

