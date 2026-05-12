#!/bin/bash
yum update -y
yum install -y docker
systemctl start docker
systemctl enable docker

# Login to ECR and pull the image
aws ecr get-login-password --region ap-south-1 | docker login --username AWS --password-stdin 609375805131.dkr.ecr.ap-south-1.amazonaws.com

# Pull and run the container
docker pull 609375805131.dkr.ecr.ap-south-1.amazonaws.com/chennai-lang-online:latest
docker run -d --restart always -p 80:3000 --name chennai-lang 609375805131.dkr.ecr.ap-south-1.amazonaws.com/chennai-lang-online:latest
