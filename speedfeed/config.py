from django.apps import AppConfig


class SpeedfeedConfig(AppConfig):
    name = 'speedfeed'
    verbose_name = 'SpeedFeed'

    def ready(self):
        from . import receivers

