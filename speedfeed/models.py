from django.db import models
from django_fsm import FSMField, transition


class Raw(models.Model):
    timestamp = models.DateTimeField()
    speed = models.FloatField()
    acc_pedal_pos = models.FloatField()
    rpm = models.FloatField()
    yawrate = models.FloatField()

    vehicle = models.ForeignKey('Vehicle')

    latitude = models.FloatField()
    longitude = models.FloatField()


class RawParticipant(models.Model):
    vel_x = models.FloatField()
    vel_y = models.FloatField()
    length = models.FloatField()
    height = models.FloatField()


class Vehicle(models.Model):
    top_speed = models.FloatField()
    last_zero_speed_time = models.DateTimeField()
    state = FSMField(default='measuring')

    @transition(field='state', source='measuring', target='measured')
    def speed_reached(self):
        pass

    @transition(field='state', source='measuring', target='measuring')
    @transition(field='state', source='measured', target='measuring')
    def stopped(self):
        pass


class ZeroToHundred(models.Model):
    time = models.DurationField()
    timestamp = models.DateTimeField()
    vehicle = models.ForeignKey('Vehicle')

