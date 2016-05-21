NAME=zubia/service-user-api
NAME_SHORT=$$(echo $(NAME) | cut -d '/' -f 2)
VERSION=$$(git rev-parse --short HEAD)
NODE_ENV=dev

install:
	@rm -rf ./node_modules
	npm install

build:
	@docker build -t $(NAME) -f docker/Dockerfile .

run: build
	@docker-compose -f docker/docker-compose.yml up

docker-build:
	docker build -t $(NAME):$(VERSION) -f docker/Dockerfile .
	docker tag -f $(NAME):$(VERSION) $(NAME):latest
	docker tag -f $(NAME):$(VERSION) $(NAME):$(BUILD_ENV) 2>/dev/null

docker-push: docker-build
	docker push $(NAME)

deploy: docker-push

.PHONY: install docker-build run docker-push deploy build
