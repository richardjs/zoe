FROM archlinux:latest

RUN pacman --noconfirm -Syu base-devel python-fastapi uvicorn && \
    useradd -u 1001 -m zoe

USER zoe
WORKDIR /home/zoe

COPY ["src", "."]
RUN make clean && \
    make

CMD ["uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8000"]
