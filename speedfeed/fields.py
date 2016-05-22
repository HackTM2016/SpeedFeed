from datetime import datetime

from rest_framework.serializers import DateTimeField

class TimestampField(DateTimeField):
    def to_internal_value(self, data):
        return datetime.utcfromtimestamp(data)

