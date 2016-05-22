from datetime import timedelta
from unittest import TestCase
from model_mommy import mommy

from django.utils import timezone

from speedfeed.models import Raw, Vehicle, ZeroToHundred
from speedfeed.serializers import RawSerializer


class RawSerializationTests(TestCase):
    def test_participants_creation(self):
        data = \
        {
            "timestamp": 123456789,
            "speed": 0.0,
            "acc_pedal_pos": 0.0,
            "rpm": 556.0,
            "yawrate": 0.0,

            "latitude": 45.748739,
            "longitude": 21.235474,

            "vehicle": 1,

            "participants": [
                {
                    "vel_x": -5.377235,
                    "vel_y": 1.828083,
                    "length": 0.0,
                    "height": 0.7
                },
                {
                    "vel_x": -5.377235,
                    "vel_y": 1.828083,
                    "length": 0.0,
                    "height": 0.7
                }
            ]
        }

        raw_object = RawSerializer(data=data)
        self.assertTrue(raw_object.is_valid())


class RawParsingTests(TestCase):
    def test_zero_to_hundred_and_top_speed(self):
        time = timezone.now()
        vehicle = mommy.make(Vehicle)
        mommy.make(Raw, speed=0, timestamp=time, vehicle=vehicle)

        vehicle.refresh_from_db()
        self.assertEqual(vehicle.top_speed, 0)
        mommy.make(Raw, speed=50, timestamp=time + timedelta(seconds=3), vehicle=vehicle)
        self.assertFalse(ZeroToHundred.objects.filter(vehicle=vehicle, timestamp=time + timedelta(seconds=3)).exists())

        vehicle.refresh_from_db()
        self.assertEqual(vehicle.top_speed, 50)

        mommy.make(Raw, speed=100, timestamp=time + timedelta(seconds=5), vehicle=vehicle)
        vehicle.refresh_from_db()
        self.assertEqual(vehicle.top_speed, 100)
        self.assertTrue(ZeroToHundred.objects.filter(vehicle=vehicle, timestamp=time + timedelta(seconds=5)).exists())

