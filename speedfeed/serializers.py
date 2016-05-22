from rest_framework import serializers
from speedfeed.models import Raw, RawParticipant
from speedfeed.fields import TimestampField


class RawParticipantsSerializer(serializers.ModelSerializer):
    class Meta:
        model = RawParticipant
        fields = '__all__'


class RawSerializer(serializers.ModelSerializer):
    timestamp = TimestampField()

    def create(self, validated_data):
        logging(validated_data)
        participants_data = validated_data.pop('participants')
        raw_object = Raw.objects.create(**validated_data)

        for participant in participants_data:
            RawParticipant.objects.create(**participant)

        return raw_object

    class Meta:
        model = Raw
        fields = '__all__'

