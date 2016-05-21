FROM python:3.5.1

MAINTAINER SpeedFeed Team

EXPOSE 8000

WORKDIR /code

ADD ./requirements.pip /code/requirements.pip
RUN pip install -r requirements.pip

ADD . /code

CMD python manage.py runserver 0.0.0.0:8000
