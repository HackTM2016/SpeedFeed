NAME=developer94/speedfeed
NAME_SHORT=$$(echo $(NAME) | cut -d '/' -f 2)
VERSION=$$(git rev-parse --short HEAD)
NODE_ENV=dev

COMPOSE=@docker-compose -f docker/docker-compose.yml

install:
	@rm -rf ./node_modules
	npm install

build:
	@docker build -t $(NAME) -f docker/Dockerfile .

build-prod:
	@docker build -t $(NAME) -f docker/Dockerfile.prod .

ps:
	$(COMPOSE) ps

up: build
	$(COMPOSE) up -d

stop:
	$(COMPOSE) stop

shell:
	$(COMPOSE) run app python manage.py shell

bash:
	$(COMPOSE) run app bash

migrations:
	$(COMPOSE) run app python manage.py makemigrations

migrate:
	$(COMPOSE) run app python manage.py migrate

test:
	$(COMPOSE) run app python manage.py test --keepdb speedfeed

docker-build:
	docker build -t $(NAME):$(VERSION) -f docker/Dockerfile .
	docker tag -f $(NAME):$(VERSION) $(NAME):latest
	docker tag -f $(NAME):$(VERSION) $(NAME):$(BUILD_ENV) 2>/dev/null

docker-push: docker-build
	docker push $(NAME)

deploy: build-prod
	eval $(docker-machine continental env)
	$(COMPOSE) up -d


.PHONY: install docker-build run docker-push deploy build
