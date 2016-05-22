from django.dispatch import receiver
from django.db.models.signals import post_save

from speedfeed.models import Raw, ZeroToHundred


@receiver(post_save, sender=Raw)
def handle_raw_data(sender, instance, raw, created, **kwargs):
    standing_speed_delta = 5
    if instance.speed > instance.vehicle.top_speed:
        instance.vehicle.top_speed = instance.speed
        instance.vehicle.save()

    if instance.speed < standing_speed_delta:
        instance.vehicle.zero_speed_time = instance.timestamp

        instance.vehicle.stopped()
        instance.vehicle.save()

    if instance.speed >= 100 and instance.vehicle.state == 'measuring':
        instance.vehicle.speed_reached()
        instance.vehicle.save()

        timediff = instance.timestamp - instance.vehicle.zero_speed_time
        ZeroToHundred.objects.create(vehicle=instance.vehicle, time=timediff, timestamp=instance.timestamp)

