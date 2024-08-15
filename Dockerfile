# Created by mars on 15/8/24

# Pull base image.
From archlinux:latest

# 修改 pacman 的镜像源
RUN sed -i 's/^#Server/Server/g' /etc/pacman.d/mirrorlist && \
    echo "Server = https://mirrors.tuna.tsinghua.edu.cn/archlinux/\$repo/os/\$arch" > /etc/pacman.d/mirrorlist && \
    echo "Server = https://mirrors.aliyun.com/archlinux/\$repo/os/\$arch" >> /etc/pacman.d/mirrorlist && \
    echo "Server = https://mirrors.ustc.edu.cn/archlinux/\$repo/os/\$arch" >> /etc/pacman.d/mirrorlist && \
    pacman -Syyu --noconfirm && \
    pacman -S --noconfirm clang gcc base-devel cmake openssl hiredis mysql-clients fmt jsoncpp

# Set environment variables.
WORKDIR /chatroom

COPY . /chatroom

CMD ["bash", "setup.sh"]
